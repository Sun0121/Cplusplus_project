/*
 *显示系统信息，包括总盘块数，盘块大小，每组盘块数，空闲块数等
 */
void info()
{
	printf("\n%15s\n","模拟Linux文件系统信息:");
	printf("%15s  %10d %-3s\n","盘块的大小:",_block_group[0].super_block.s_log_block_size,"字节");
	printf("%15s  %10d %-3s\n","每组的盘块数:",_block_group[0].super_block.s_blocks_per_group,"块");
	printf("%15s  %10d %-3s\n","总的盘块数:",_block_group[0].super_block.s_blocks_count,"块");
	printf("%15s  %10d %-3s\n","空闲块的总数:",_block_group[0].super_block.s_free_blocks_count,"块");
	printf("%15s  %10d %-3s\n","总容量为:",_block_group[0].super_block.s_blocks_count*BLOCK_SIZE,"字节");
	printf("%15s  %10d %-3s\n","剩余空间为:",_block_group[0].super_block.s_free_blocks_count*BLOCK_SIZE,"字节");
}