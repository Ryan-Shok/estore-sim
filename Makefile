BUILD := build

EXTRA_CFLAGS ?=

CC	:= gcc
CPP     := g++ -pipe
CFLAGS	:= -MD -I. -Wall -g -c $(EXTRA_CFLAGS)
LDFLAGS := -lpthread -lrt

SIM_OBJS	:=	estoresim.o 		\
    			TaskQueue.o		\
			EStore.o		\
			RequestGenerator.o	\
			RequestHandlers.o	\
			sthread.o

SIM_OBJS	:= $(patsubst %.o,$(BUILD)/%.o,$(SIM_OBJS))

all: $(BUILD)/estoresim
	@:


$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CPP) $(CFLAGS) $< -o $@ 

$(BUILD)/estoresim: $(SIM_OBJS)
	$(CPP) -o $@ $(SIM_OBJS) $(LDFLAGS)

-include $(BUILD)/*.d

clean:
	rm -rf $(BUILD)

.PHONY: clean always

always: 
	@:

realclean: clean
	@echo + realclean
	$(V)rm -rf $(DISTDIR) $(DISTDIR).tgz


tidy: always
	git clean -dff

run-sim: $(BUILD)/estoresim always
	build/estoresim

run-sim-fine: $(BUILD)/estoresim always
	build/estoresim --fine
