/*
 * Copyright (c) [2004-2009] Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */


#include <sstream>

#include "storage/Dmraid.h"
#include "storage/DmraidCo.h"
#include "storage/Utils/SystemCmd.h"
#include "storage/Utils/AppUtil.h"
#include "storage/Storage.h"


namespace storage
{
    using namespace std;


    Dmraid::Dmraid(const DmraidCo& c, const string& name, const string& device, unsigned nr,
		   Partition* p)
	: DmPart(c, name, device, nr, p)
    {
	y2mil("constructed Dmraid " << dev << " on " << cont->device());
    }

    Dmraid::Dmraid(const DmraidCo& c, const string& name, const string& device, unsigned nr,
		   Partition* p, SystemInfo& si)
	: DmPart(c, name, device, nr, p, si)
    {
	y2mil("constructed Dmraid " << dev << " on " << cont->device());
    }


    Dmraid::Dmraid(const DmraidCo& c, const Dmraid& v)
	: DmPart(c, v)
    {
	y2deb("copy-constructed Dmraid from " << v.dev);
    }


    Dmraid::~Dmraid()
    {
	y2deb("destructed Dmraid " << dev);
    }


Text Dmraid::removeText( bool doing ) const
    {
    Text txt;
    string d = dev.substr(12);
    if( p && p->OrigNr()!=p->nr() )
	d = co()->getPartName(p->OrigNr());
    if( doing )
	{
	// displayed text during action, %1$s is replaced by raid partition name e.g. pdc_dabaheedj_part1
	txt = sformat( _("Deleting RAID partition %1$s"), d.c_str() );
	}
    else
	{
	// displayed text before action, %1$s is replaced by raid partition name e.g. pdc_dabaheedj_part1
	// %2$s is replaced by size (e.g. 623.5 MB)
	txt = sformat( _("Delete RAID partition %1$s (%2$s)"), d.c_str(),
		       sizeString().c_str() );
	}
    return( txt );
    }

Text Dmraid::createText( bool doing ) const
    {
    Text txt;
    string d = dev.substr(12);
    if( doing )
	{
	// displayed text during action, %1$s is replaced by raid partition name e.g. pdc_dabaheedj_part1
	txt = sformat( _("Creating RAID partition %1$s"), d.c_str() );
	}
    else
	{
	if( mp=="swap" )
	    {
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Create swap RAID partition %1$s (%2$s)"),
	                   d.c_str(), sizeString().c_str() );
	    }
	else if( !mp.empty() )
	    {
	    if( encryption==ENC_NONE )
		{
		// displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Create RAID partition %1$s (%2$s) for %4$s with %3$s"),
			       d.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    else
		{
		// displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Create encrypted RAID partition %1$s (%2$s) for %4$s with %3$s"),
			       d.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    }
	else if( p && p->type()==EXTENDED )
	    {
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Create extended RAID partition %1$s (%2$s)"),
			   d.c_str(), sizeString().c_str() );
	    }
	else
	    {
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Create RAID partition %1$s (%2$s)"),
			   d.c_str(), sizeString().c_str() );
	    }
	}
    return( txt );
    }

Text Dmraid::formatText( bool doing ) const
    {
    Text txt;
    string d = dev.substr(12);
    if( doing )
	{
	// displayed text during action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	// %2$s is replaced by size (e.g. 623.5 MB)
	// %3$s is replaced by file system type (e.g. reiserfs)
	txt = sformat( _("Formatting RAID partition %1$s (%2$s) with %3$s"),
		       d.c_str(), sizeString().c_str(), fsTypeString().c_str() );
	}
    else
	{
	if( !mp.empty() )
	    {
	    if( mp=="swap" )
		{
		// displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
		// %2$s is replaced by size (e.g. 623.5 MB)
		txt = sformat( _("Format RAID partition %1$s (%2$s) for swap"),
			       d.c_str(), sizeString().c_str() );
		}
	    else if( encryption==ENC_NONE )
		{
		// displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Format RAID partition %1$s (%2$s) for %4$s with %3$s"),
			       d.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    else
		{
		// displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
		// %2$s is replaced by size (e.g. 623.5 MB)
		// %3$s is replaced by file system type (e.g. reiserfs)
		// %4$s is replaced by mount point (e.g. /usr)
		txt = sformat( _("Format encrypted RAID partition %1$s (%2$s) for %4$s with %3$s"),
			       d.c_str(), sizeString().c_str(), fsTypeString().c_str(),
			       mp.c_str() );
		}
	    }
	else
	    {
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    // %3$s is replaced by file system type (e.g. reiserfs)
	    txt = sformat( _("Format RAID partition %1$s (%2$s) with %3$s"),
			   d.c_str(), sizeString().c_str(),
			   fsTypeString().c_str() );
	    }
	}
    return( txt );
    }

Text Dmraid::resizeText( bool doing ) const
    {
    Text txt;
    string d = dev.substr(12);
    if( doing )
        {
	if( needShrink() )
	    // displayed text during action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Shrinking RAID partition %1$s to %2$s"), d.c_str(), sizeString().c_str() );
	else
	    // displayed text during action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Extending RAID partition %1$s to %2$s"), d.c_str(), sizeString().c_str() );
	txt += Text(" ", " ");
	// text displayed during action
	txt += _("(Progress bar will not move. May take very long. DO NOT ABORT!)");
        }
    else
        {
	if( needShrink() )
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Shrink RAID partition %1$s to %2$s"), d.c_str(), sizeString().c_str() );
	else
	    // displayed text before action, %1$s is replaced by raid partition e.g. pdc_dabaheedj_part1
	    // %2$s is replaced by size (e.g. 623.5 MB)
	    txt = sformat( _("Extend RAID partition %1$s to %2$s"), d.c_str(), sizeString().c_str() );

        }
    return( txt );
    }

Text Dmraid::setTypeText( bool doing ) const
    {
    Text txt;
    string d = dev.substr(12);
    if( doing )
        {
        // displayed text during action, %1$s is replaced by partition name (e.g. pdc_dabaheedj_part1),
        // %2$s is replaced by hexadecimal number (e.g. 8E)
        txt = sformat( _("Setting type of RAID partition %1$s to %2$X"),
                      d.c_str(), id() );
        }
    else
        {
        // displayed text before action, %1$s is replaced by partition name (e.g. pdc_dabaheedj_part1),
        // %2$s is replaced by hexadecimal number (e.g. 8E)
        txt = sformat( _("Set type of RAID partition %1$s to %2$X"),
                      d.c_str(), id() );
        }
    return( txt );
    }

void Dmraid::getInfo( DmraidInfo& info ) const
    {
    DmPart::getInfo( info.p );
    }


std::ostream& operator<< (std::ostream& s, const Dmraid &p )
    {
    s << dynamic_cast<const DmPart&>(p);
    return( s );
    }


bool Dmraid::equalContent( const Dmraid& rhs ) const
    {
    return( DmPart::equalContent(rhs) );
    }


    void
    Dmraid::logDifference(std::ostream& log, const Dmraid& rhs) const
    {
	DmPart::logDifference(log, rhs);
    }

}
