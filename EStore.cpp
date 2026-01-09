#include <cassert>

#include "EStore.h"
#include "sthread.h"
#include <algorithm>
#include <cstdio>
#include <vector>

using namespace std;

Item::
Item() : valid(false)
{ }

Item::
~Item()
{ }


EStore::
EStore(bool enableFineMode)
    : fineMode(enableFineMode)
{
    smutex_init(&mutex);
    scond_init(&cond);
    for (int i = 0; i < INVENTORY_SIZE; i++)
    {
        smutex_init(&fineMutexes[i]);
    }
    smutex_init(&shippingLock);
    smutex_init(&discountLock);
    storeDiscount = 0;
    shippingCost = 3.0;
}

EStore::
~EStore()
{
    smutex_destroy(&mutex);
    scond_destroy(&cond);
    for (int i = 0; i < INVENTORY_SIZE; i++)
    {
        smutex_destroy(&fineMutexes[i]);
    }
    smutex_destroy(&shippingLock);
    smutex_destroy(&discountLock);
}

/*
 * ------------------------------------------------------------------
 * buyItem --
 *
 *      Attempt to buy the item from the store.
 *
 *      An item can be bought if:
 *          - The store carries it.
 *          - The item is in stock.
 *          - The cost of the item plus the cost of shipping is no
 *            more than the budget.
 *
 *      If the store *does not* carry this item, simply return and
 *      do nothing. Do not attempt to buy the item.
 *
 *      If the store *does* carry the item, but it is not in stock
 *      or its cost is over budget, block until both conditions are
 *      met (at which point the item should be bought) or the store
 *      removes the item from sale (at which point this method
 *      returns).
 *
 *      The overall cost of a purchase for a single item is defined
 *      as the current cost of the item times 1 - the store
 *      discount, plus the flat overall store shipping fee.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyItem(int item_id, double budget)
{
    assert(!fineModeEnabled());
    
    // only allow one thread to buy an item at a time
    smutex_lock(&mutex);
    // check for valid
    if (!inventory[item_id].valid)
    {
        // return when not valid
        smutex_unlock(&mutex);
        return;
    }
    // wait until quantity is not zero and cost does not exceed budget (make sure still valid)
    while (inventory[item_id].valid && (inventory[item_id].quantity == 0 || (inventory[item_id].price * (1 - inventory[item_id].discount) 
        * (1 - storeDiscount) + shippingCost) > budget))
    {
        scond_wait(&cond, &mutex);
    }

    // check for valid
    if (!inventory[item_id].valid)
    {
        smutex_unlock(&mutex);
        return;
    }
    else
    {
        // buy the item
        inventory[item_id].quantity -= 1;
        smutex_unlock(&mutex);
    }
}

/*
 * ------------------------------------------------------------------
 * buyManyItem --
 *
 *      Attempt to buy all of the specified items at once. If the
 *      order cannot be bought, give up and return without buying
 *      anything. Otherwise buy the entire order at once.
 *
 *      The entire order can be bought if:
 *          - The store carries all items.
 *          - All items are in stock.
 *          - The cost of the the entire order (cost of items plus
 *            shipping for each item) is no more than the budget.
 *
 *      If multiple customers are attempting to buy at the same
 *      time and their orders are mutually exclusive (i.e., the
 *      two customers are not trying to buy any of the same items),
 *      then their orders must be processed at the same time.
 *
 *      The cost of a purchase of many items is the sum of the
 *      costs of purchasing each item individually. The purchase
 *      cost of an individual item is covered above in the
 *      description of buyItem.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyManyItems(vector<int>* item_ids, double budget)
{
    assert(fineModeEnabled());
    // check if there are no items and return if so
    if (item_ids -> empty())
    {
    	return;
    }
    // sort the items in ascending order to avoid deadlock buy locking in a specified order
    sort(item_ids->begin(), item_ids->end(), greater<int>());

    // keep track of total cost for all items
    double totalCost = 0.0;
    for (int i = 0; (size_t)i < item_ids->size(); i++)
    {
        // attempt to lock every item
        smutex_lock(&fineMutexes[(*item_ids)[i]]);
        // check for valid and quantity above 0
        if (!inventory[(*item_ids)[i]].valid || inventory[(*item_ids)[i]].quantity == 0)
        {
            // unlock all previous locks if not valid or out of stock
            for (; i >= 0; i--)
            {
                smutex_unlock(&fineMutexes[(*item_ids)[i]]);
            }
            return;
        }
        // update total cost if valid or not out of stock
        else
        {
            totalCost += inventory[(*item_ids)[i]].price * (1 - inventory[(*item_ids)[i]].discount);
        }
    }

    // check if exceeds budget
    if (totalCost * (1 - storeDiscount) + shippingCost * item_ids->size() > budget)
    {
        for (int j = 0; (size_t)j < item_ids->size(); j++)
        {
            smutex_unlock(&fineMutexes[(*item_ids)[j]]);
        }   
    }
    // buy if all items together don't exceed budget
    else
    {
        for (int j = 0; (size_t)j < item_ids->size(); j++)
        {
            inventory[(*item_ids)[j]].quantity -= 1;
            smutex_unlock(&fineMutexes[(*item_ids)[j]]);
        }
    }
    return;
}

/*
 * ------------------------------------------------------------------
 * addItem --
 *
 *      Add the item to the store with the specified quantity,
 *      price, and discount. If the store already carries an item
 *      with the specified id, do nothing.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addItem(int item_id, int quantity, double price, double discount)
{
    if (!fineModeEnabled())
    {
        // only allow for one thread to access shared state
        smutex_lock(&mutex);
        // check for valid
        if (inventory[item_id].valid)
        {
            smutex_unlock(&mutex);
            return;
        }

        // create new item if not already valid
        Item item;
        item.quantity = quantity;
        item.price = price;
        item.discount = discount;
        item.valid = true;
        inventory[item_id] = item;

        smutex_unlock(&mutex);
    }
    else
    {
        // allow only one thread to access this specific item
        smutex_lock(&fineMutexes[item_id]);
        // check for valid
        if (inventory[item_id].valid)
        {
            smutex_unlock(&fineMutexes[item_id]);
            return;
        }

        // create new item if not already valid
        Item item;
        item.quantity = quantity;
        item.price = price;
        item.discount = discount;
        item.valid = true;
        inventory[item_id] = item;

        smutex_unlock(&fineMutexes[item_id]);
    }

    return;

}

/*
 * ------------------------------------------------------------------
 * removeItem --
 *
 *      Remove the item from the store. The store no longer carries
 *      this item. If the store is not carrying this item, do
 *      nothing.
 *
 *      Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
removeItem(int item_id)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&mutex);
            return;
        }

        // set the items validity to false to remove it
        inventory[item_id].valid = false;

        // wake any waiters
        scond_broadcast(&cond, &mutex);
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access this item
        smutex_lock(&fineMutexes[item_id]);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&fineMutexes[item_id]);
            return;
        }

        // set the items validity to false to remove it
        inventory[item_id].valid = false;

        smutex_unlock(&fineMutexes[item_id]);
    }

    return;
}

/*
 * ------------------------------------------------------------------
 * addStock --
 *
 *      Increase the stock of the specified item by count. If the
 *      store does not carry the item, do nothing. Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addStock(int item_id, int count)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&mutex);
            return;
        }

        // add more stock if valid
        inventory[item_id].quantity += count;

        scond_broadcast(&cond, &mutex);
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access this item
        smutex_lock(&fineMutexes[item_id]);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&fineMutexes[item_id]);
            return;
        }

        // add more stock if valid
        inventory[item_id].quantity += count;

        smutex_unlock(&fineMutexes[item_id]);
    }

    return;
}

/*
 * ------------------------------------------------------------------
 * priceItem --
 *
 *      Change the price on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item price decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
priceItem(int item_id, double price)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&mutex);
            return;
        }
        // change price if valid
        double oldPrice = inventory[item_id].price;
        inventory[item_id].price = price;

        // if price decreased, wake waiters
        if (oldPrice > price)
        {
            scond_broadcast(&cond, &mutex);
        }
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access this item
        smutex_lock(&fineMutexes[item_id]);

        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&fineMutexes[item_id]);
            return;
        }
        // change the price if valid
        inventory[item_id].price = price;

        smutex_unlock(&fineMutexes[item_id]);
    }
    return;
}

/*
 * ------------------------------------------------------------------
 * discountItem --
 *
 *      Change the discount on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item discount increased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
discountItem(int item_id, double discount)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&mutex);
            return;
        }
        // change discount if valid
        double oldDiscount = inventory[item_id].discount;
        inventory[item_id].discount = discount;

        // wake waiters if discount increased
        if (oldDiscount < discount)
        {
            scond_broadcast(&cond, &mutex);
        }
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access this item
        smutex_lock(&fineMutexes[item_id]);

        // check for valid
        if (!inventory[item_id].valid)
        {
            smutex_unlock(&fineMutexes[item_id]);
            return;
        }
        // change discount if valid
        inventory[item_id].discount = discount;

        smutex_unlock(&fineMutexes[item_id]);
    }

    return;
}

/*
 * ------------------------------------------------------------------
 * setShippingCost --
 *
 *      Set the per-item shipping cost. If the shipping cost
 *      decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setShippingCost(double cost)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        // change the shipping cost if valid
        double oldShippingCost = shippingCost;
        shippingCost = cost;

        // wake any waiters if shipping decreased
        if (oldShippingCost > cost)
        {
            scond_broadcast(&cond, &mutex);
        }
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access the shipping cost
        smutex_lock(&shippingLock);

        // change the shipping cost
        shippingCost = cost;

        smutex_unlock(&shippingLock);
    }

    return;
}

/*
 * ------------------------------------------------------------------
 * setStoreDiscount --
 *
 *      Set the store discount. If the discount increased, wake any
 *      waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setStoreDiscount(double discount)
{
    if (!fineModeEnabled())
    {
        // only allow one thread to access the shared state
        smutex_lock(&mutex);
        
        // change the discount
        double oldStoreDiscount = storeDiscount;
        storeDiscount = discount;

        // wake any waiters if the store discount increased
        if (oldStoreDiscount < storeDiscount)
        {
            scond_broadcast(&cond, &mutex);
        }
        smutex_unlock(&mutex);
    }
    else
    {
        // only allow one thread to access the store discount
        smutex_lock(&discountLock);

        // change the store discount
        storeDiscount = discount;

        smutex_unlock(&discountLock);
    }

    return;
}
