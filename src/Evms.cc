#include <sstream>

#include "y2storage/Evms.h"
#include "y2storage/EvmsCo.h"
#include "y2storage/SystemCmd.h"
#include "y2storage/AppUtil.h"
#include "y2storage/Storage.h"

using namespace storage;
using namespace std;

Evms::Evms( const EvmsCo& d, const string& name, unsigned long le ) :
	Dm( d, d.name().empty()?name:d.name()+"/"+name )
    {
    init( name );
    setLe( le );
    calcSize();
    getTableInfo();
    y2milestone( "constructed evms vol %s on vg %s", dev.c_str(),
                 cont->name().c_str() );
    }

Evms::Evms( const EvmsCo& d, const string& name, unsigned long le,
	    unsigned str ) :
	Dm( d, d.name()+"-"+name )
    {
    init( name );
    setLe( le );
    calcSize();
    stripe = str;
    fs = detected_fs = FSNONE;
    y2milestone( "constructed evms vol %s on vg %s", dev.c_str(),
                 cont->name().c_str() );
    }

Evms::~Evms()
    {
    y2milestone( "destructed evms vol %s", dev.c_str() );
    }

void Evms::init( const string& name )
    {
    compat = true;
    nm = name;
    dev = "/dev/evms" + cont->name() + "/" + name;
    Dm::init();
    }

string Evms::removeText( bool doing ) const
    {
    string txt;
    if( doing )
	{
	// displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	txt = sformat( _("Deleting EVMS volume %1$s"), dev.c_str() );
	}
    else
	{
	// displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	// %2$s is replaced by size (e.g. 623.5 MB)
	txt = sformat( _("Delete EVMS volume %1$s %2$s"), dev.c_str(),
		       sizeString().c_str() );
	}
    return( txt );
    }

string Evms::createText( bool doing ) const
    {
    string txt;
    if( doing )
	{
	// displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	txt = sformat( _("Creating EVMS volume %1$s"), dev.c_str() );
	}
    else
	{
	if( mp=="swap" )
	    {
	    // displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Create swap EVMS volume %1$s %2$s"),
	                   dev.c_str(), sizeString().c_str() );
	    }
	else if( !mp.empty() )
	    {
	    if( encryption==ENC_NONE )
		{
		// displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Create EVMS volume %1$s %2$s for %4$s with %3$s"),
			       dev.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    else
		{
		// displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Create crypted EVMS volume %1$s %2$s for %4$s with %3$s"),
			       dev.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    }
	else
	    {
	    // displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Create EVMS volume %1$s %2$s"),
			   dev.c_str(), sizeString().c_str() );
	    }
	}
    return( txt );
    }

string Evms::formatText( bool doing ) const
    {
    string txt;
    if( doing )
	{
	// displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	// %2$s is replaced by size (e.g. 623.5 MB)
	// %3$s is replaced by file system type (e.g. reiserfs)
	txt = sformat( _("Formatting EVMS volume %1$s %2$s with %3$s "),
		       dev.c_str(), sizeString().c_str(), fsTypeString().c_str() );
	}
    else
	{
	if( !mp.empty() )
	    {
	    if( encryption==ENC_NONE )
		{
		// displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Format EVMS volume %1$s %2$s for %4$s with %3$s"),
			       dev.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    else
		{
		// displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Format crypted EVMS volume %1$s %2$s for %4$s with %3$s"),
			       dev.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    }
	else
	    {
	    // displayed text before action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    // %3$s is replaced by file system type (e.g. reiserfs)
	    txt = sformat( _("Format EVMS volume %1$s %2$s with %3$s"),
			   dev.c_str(), sizeString().c_str(), 
			   fsTypeString().c_str() );
	    }
	}
    return( txt );
    }

string Evms::resizeText( bool doing ) const
    {
    string txt;
    if( doing )
        {
	if( needShrink() )
	    // displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Shrinking EVMS volume %1$s to %2$s"), dev.c_str(), sizeString().c_str() );
	else
	    // displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Extending EVMS volume %1$s to %2$s"), dev.c_str(), sizeString().c_str() );

        }
    else
        {
	if( needShrink() )
	    // displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Shrink EVMS volume %1$s to %2$s"), dev.c_str(), sizeString().c_str() );
	else
	    // displayed text during action, %1$s is replaced by device name e.g. /dev/evms/lvm/system/usr
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Extend EVMS volume %1$s to %2$s"), dev.c_str(), sizeString().c_str() );

        }
    return( txt );
    }
