How to insert module at boot :
1) sudo cp PATH_TO_MODULE/MODULE_NAME.ko /lib/modules/$(uname -r)/kernel/drivers/

2) echo 'MODULE_NAME' | sudo tee -a /etc/modules

3) sudo depmod
