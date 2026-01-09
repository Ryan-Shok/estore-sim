#include "Request.h"
#include "EStore.h"
#include "RequestHandlers.h"
#include "sthread.h"
#include <cstdlib>
#include <cstdio>

/*
 * ------------------------------------------------------------------
 * add_item_handler --
 *
 *      Handle an AddItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
add_item_handler(void* args)
{
    struct AddItemReq* req = (AddItemReq*)args;
    
    // handle task by calling respective EStore method
    req->store->addItem(req->item_id, req->quantity, req->price, req->discount);
    // print arguments info
    printf("Handling AddItemReq: item_id - %d, quantity - %d, price - $%.2f, discount - %.2f\n", req->item_id, req->quantity, req->price, req->discount);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * remove_item_handler --
 *
 *      Handle a RemoveItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
remove_item_handler(void* args)
{
    struct RemoveItemReq* req = (RemoveItemReq*)args;

    // handle task by calling respective EStore method
    req->store->removeItem(req->item_id);
    // print arguments info
    printf("Handling RemoveItemReq: item_id - %d\n", req->item_id);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * add_stock_handler --
 *
 *      Handle an AddStockReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
add_stock_handler(void* args)
{
    struct AddStockReq* req = (AddStockReq*)args;
    
    // handle task by calling respective EStore method
    req->store->addStock(req->item_id, req->additional_stock);
    // print arguments info
    printf("Handling AddStockReq: item_id - %d, additional_stock - %d\n", req->item_id, req->additional_stock);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * change_item_price_handler --
 *
 *      Handle a ChangeItemPriceReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
change_item_price_handler(void* args)
{
    struct ChangeItemPriceReq* req = (ChangeItemPriceReq*)args;

    // handle task by calling respective EStore method
    req->store->priceItem(req->item_id, req->new_price);
    // print arguments info
    printf("Handling ChangeItemPriceReq: item_id - %d, new_price - $%.2f\n", req->item_id, req->new_price);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * change_item_discount_handler --
 *
 *      Handle a ChangeItemDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
change_item_discount_handler(void* args)
{
    struct ChangeItemDiscountReq* req = (ChangeItemDiscountReq*)args;

    // handle task by calling respective EStore method
    req->store->discountItem(req->item_id, req->new_discount);
    // print arguments info
    printf("Handling ChangeItemDiscountReq: item_id - %d, new_discount - %.2f\n", req->item_id, req->new_discount);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * set_shipping_cost_handler --
 *
 *      Handle a SetShippingCostReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
set_shipping_cost_handler(void* args)
{
    struct SetShippingCostReq* req = (SetShippingCostReq*)args;

    // handle task by calling respective EStore method
    req->store->setShippingCost(req->new_cost);
    // print arguments info
    printf("Handling SetShippingCostReq: new_shipping - $%.2f\n", req->new_cost);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * set_store_discount_handler --
 *
 *      Handle a SetStoreDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
set_store_discount_handler(void* args)
{
    struct SetStoreDiscountReq* req = (SetStoreDiscountReq*)args;

    // handle task by calling respective EStore method
    req->store->setStoreDiscount(req->new_discount);
    // print arguments info
    printf("Handling SetStoreDiscountReq: new_discount - %.2f\n", req->new_discount);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * buy_item_handler --
 *
 *      Handle a BuyItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_item_handler(void* args)
{
    struct BuyItemReq* req = (BuyItemReq*)args;

    // handle task by calling respective EStore method
    req->store->buyItem(req->item_id, req->budget);
    // print arguments info
    printf("Handling BuyItemReq: item_id - %d, budget - $%.2f\n", req->item_id, req->budget);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * buy_many_items_handler --
 *
 *      Handle a BuyManyItemsReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_many_items_handler(void* args)
{
    struct BuyManyItemsReq* req = (BuyManyItemsReq*)args;

    // handle task by calling respective EStore method
    req->store->buyManyItems(&req->item_ids, req->budget);
    // print arguments info
    printf("Handling BuyManyItemsReq: item_ids - ");
    for (size_t i = 0; i < req->item_ids.size(); i++)
    {
        printf("%d ", req->item_ids[i]);
    }
    printf(", budget - $%.2f\n", req->budget);
    free(req);
    // TODO: Your code here.
}

/*
 * ------------------------------------------------------------------
 * stop_handler --
 *
 *      The thread should exit.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
stop_handler(void* args)
{
    // print info that you are stopping the calling thread
    printf("Handling StopHandlerReq : Quitting.\n");
    sthread_exit();
    // TODO: Your code here.
}
