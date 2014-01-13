
#include <iostream>

#include "common.h"

#include "storage/Parted.h"


using namespace std;
using namespace storage;


void
parse1()
{
    cout << "parse1" << endl;

    vector<string> lines = {
	"Model: ATA WDC WD10EADS-00M (scsi)",
	"Disk /dev/sda: 121601cyl",
	"Sector size (logical/physical): 512B/4096B",
	"BIOS cylinder,head,sector geometry: 121601,255,63.  Each cylinder is 8225kB.",
	"Partition Table: msdos",
	"",
	"Number  Start   End        Size       Type     File system  Flags",
	" 1      0cyl    131cyl     130cyl     primary  ext3         boot, type=83",
	" 2      131cyl  117618cyl  117487cyl  primary               lvm, type=8e",
	"",
	"Model: ATA WDC WD10EADS-00M (scsi)",
	"Disk /dev/sda: 1953525168s",
	"Sector size (logical/physical): 512B/4096B",
	"Partition Table: msdos",
	"",
	"Number  Start     End          Size         Type     File system  Flags",
	" 1      2048s     2105343s     2103296s     primary  ext3         boot, type=83",
	" 2      2105344s  1889548287s  1887442944s  primary               lvm, type=8e",
	""
    };

    Parted parted("/dev/sda", false);
    parted.parse(lines);

    cout << parted << endl;
}


int
main()
{
    cout.setf(std::ios::boolalpha);

    setup_logger();

    parse1();
}
