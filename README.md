# wc24-data-removal-tool
Wipes the WiiConnect24 cache data from the NAND.

This tool aids in the recovery from a Mailbox Brick, which can occur if the WiiConnect24 cache files stored at /shared2/wc24/ get corrupted. The application accomplishes this by deleting those files using ISFS instructions + full hardware access through AHB and then rebooting the Wii. The cache data automatically gets rebuilt by the System Menu in the next boot, thus fixing the problem.