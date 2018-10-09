.PHONY: all clean

SUBDIRS=libencoder libamazon libstream main
	
all:
	$(foreach D, $(SUBDIRS), make -C $(D) all;)
	cp -rf ./out/ipc18.bin /mnt/hgfs/vmware_share/
	
clean:
	$(foreach D, $(SUBDIRS), make -C $(D) clean;)
	
