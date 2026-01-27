# List of subdirectories that contain their own Makefile
SUBDIRS := manwb hist res crop

.PHONY: all $(SUBDIRS) clean

# Default target builds all subdirs
all: $(SUBDIRS)

# "make" in each subdirectory
$(SUBDIRS):
	$(MAKE) -C $@

# Clean everything
clean:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d clean; \
	done
