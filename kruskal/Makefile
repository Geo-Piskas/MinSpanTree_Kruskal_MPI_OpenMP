SRCSER=src/serial
SRCMPI=src/mpi

BIN=bin/
BUILD=build/

.PHONY: clean makedirs all $(SRCSER) $(SRCMPI)

all: clean makedirs serial mpi

serial:
	$(MAKE) -C $(SRCSER)

mpi:
	$(MAKE) -C $(SRCMPI)

makedirs:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -rf $(BIN) $(BUILD)
