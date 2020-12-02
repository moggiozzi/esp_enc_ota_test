BUG reproduce steps:
1) Write key & efuse to device: run dev_enc_init.bat com<PORT_NUM> my_key.bin
3) Write files to device: fw_write.bat com<PORT_NUM> ./build/bootloader/bootloader.en ./build/partition_table/partition-table.en ./build/bt_spp_acceptor_demo0_v8.en ./build/bt_spp_acceptor_demo1_v8.en
4) Run & show output: idf.py monitor -p com<PORT_NUM>

BUG reproduce steps (alternative with build):
1) Write key & efuse to device: run dev_enc_init.bat com<PORT_NUM> my_key.bin
2) build.bat
3) build_encrypted.bat
4) Write files to device: fw_write.bat com<PORT_NUM> ./build/bootloader/bootloader.en ./build/partition_table/partition-table.en ./build/bt_spp_acceptor_demo0_v8.en ./build/bt_spp_acceptor_demo1_v8.en
	WARNING: change file names in script params
5) Run & show output: idf.py monitor -p com<PORT_NUM>

Sample output files in "logs" directory.