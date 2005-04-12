#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <list>

#include "y2storage/StorageInterface.h"
#include "y2storage/StorageTypes.h"
#include "y2storage/StorageTmpl.h"
#include "y2storage/Container.h"
#include "y2storage/Disk.h"
#include "y2storage/Volume.h"
#include "y2storage/Partition.h"
#include "y2storage/LvmVg.h"
#include "y2storage/LvmLv.h"
#include "y2storage/Evms.h"
#include "y2storage/EvmsVol.h"
#include "y2storage/Md.h"
#include "y2storage/Loop.h"
#include "y2storage/FilterIterator.h"
#include "y2storage/DerefIterator.h"
#include "y2storage/ListListIterator.h"
#include "y2storage/IterPair.h"

using namespace std;

template <int Value>
class CheckType 
    {
    public:
	bool operator()( const Container& d ) const
	    {
	    return( d.type()==Value );
	    }
    };

template< class Iter, int Value, class CastResult >
class CastCheckIterator : public CheckType<Value>, 
                          public FilterIterator< CheckType<Value>, Iter >
    {
    typedef FilterIterator<CheckType<Value>, Iter> _bclass;
    public:
	typedef CastResult value_type;
	typedef CastResult& reference;
	typedef CastResult* pointer;

	CastCheckIterator() : _bclass() {}
	CastCheckIterator( const Iter& b, const Iter& e, bool atend=false) : 
	    _bclass( b, e, *this, atend ) {}
	CastCheckIterator( const IterPair<Iter>& pair, bool atend=false) : 
	    _bclass( pair, *this, atend ) {}
	CastCheckIterator( const CastCheckIterator& i) { *this=i;}
	CastResult operator*() const
	    {
	    return( static_cast<CastResult>(_bclass::operator*()) );
	    }
	CastResult* operator->() const
	    {
	    return( static_cast<CastResult*>(_bclass::operator->()) );
	    }
	CastCheckIterator& operator++() 
	    { 
	    _bclass::operator++(); return(*this); 
	    }
	CastCheckIterator operator++(int) 
	    { 
	    y2warning( "Expensive ++ CastCheckIterator" );
	    CastCheckIterator tmp(*this);
	    _bclass::operator++(); 
	    return(tmp); 
	    }
	CastCheckIterator& operator--() 
	    { 
	    _bclass::operator--(); return(*this); 
	    }
	CastCheckIterator operator--(int) 
	    { 
	    y2warning( "Expensive -- CastCheckIterator" );
	    CastCheckIterator tmp(*this);
	    _bclass::operator--(); 
	    return(tmp); 
	    }
    };

/**
 * \brief Main class to access libstorage functionality.
 *
 * This is the main class with that one can get access to the
 * functionality provided by libstorage.
 * It contains a list of container objects.
 *
 * All modifying member functions of the storage library will
 * go through Storage class. This is the central place where
 * things like readonly access, locking, testmode, inst-sys etc.
 * are handled. It has the additional advantage the the complete
 * class hierarchy below Storage could be changed without affecting
 * the user interface of libstorage.
 */

class EtcFstab;

class Storage : public StorageInterface
    {
    protected:

	typedef list<Container*> CCont;
	typedef CCont::iterator CIter;
	typedef CCont::const_iterator CCIter;

	static bool isMd( const Container&d )
	    { return( d.type()==MD ); };
	static bool isLoop( const Container&d )
	    { return( d.type()==LOOP ); };
    public:
	struct SkipDeleted { bool operator()(const Container&d) const {return( !d.deleted());}};
	static SkipDeleted SkipDel;
	static bool notDeleted( const Container&d ) { return( !d.deleted() ); };

	Storage( bool ronly=false, bool testmode=false, bool autodetect=true );
	bool test() const { return( testmode ); }
	bool instsys() const { return( inst_sys ); }
	void setCacheChanges( bool val=true ) { cache = val; }
	bool isCacheChanges() const { return( cache ); }
	void assertInit() { if( !initialized ) initialize(); }
	int checkCache();
	const string& tDir() const { return( testdir ); }
	const string& root() const { return( rootprefix ); }
	const string& tmpDir() const { return( tempdir ); }
	static const string& arch() { return( proc_arch ); }
	EtcFstab* getFstab() { return fstab; }
	void handleLogFile( const string& name );
	static bool testFilesEqual( const string& n1, const string& n2 );
	void printInfo( ostream& str );
	bool setUsedBy( const string& dev, UsedByType typ, const string& name );

	virtual ~Storage();

	// functions for interface

	bool getDisks (list<string>& disks);
	bool getPartitions (list<PartitionInfo>& partitioninfos);
	bool getPartitions (const string& disk, list<PartitionInfo>& partitioninfos);
	bool getFsCapabilities (FsType fstype, FsCapabilities& fscapabilities);
	int createPartition( const string& disk, PartitionType type, unsigned long start,
			     unsigned long size, string& device );
	int createPartitionKb( const string& disk, PartitionType type,
	                       unsigned long long start,
			       unsigned long long sizek, string& device );
	unsigned long kbToCylinder( const string& disk, unsigned long long size );
	unsigned long long cylinderToKb( const string& disk, unsigned long size );
	int removePartition( const string& partition );
	int changePartitionId( const string& partition, unsigned id );
	int destroyPartitionTable( const string& disk, const string& label );
	string defaultDiskLabel();

	int changeFormatVolume( const string&, bool format, FsType fs );
	int changeMountPoint( const string&, const string& mount );
	int getMountPoint( const string&, string& mount );
	int changeMountBy( const string& device, MountByType mby );
	int getMountBy( const string& device, MountByType& mby );
	int changeFstabOptions( const string&, const string& options );
	int getFstabOptions( const string& device, string& options );
	int addFstabOptions( const string&, const string& options );
	int removeFstabOptions( const string&, const string& options );
	int setCryptPassword( const string& device, const string& pwd );
	int setCrypt( const string& device, bool val );
	int getCrypt( const string& device, bool& val );
	int resizeVolume( const string& device, unsigned long long newSizeMb );

	int createLvmVg( const string& name, bool lvm1,
			 const list<string>& devs );
	int extendLvmVg( const string& name, const list<string>& devs );
	int shrinkLvmVg( const string& name, const list<string>& devs );
	int createLvmLv( const string& vg, const string& name,
			 unsigned long long sizeM, unsigned stripe=1 );
	int removeLvmLv( const string& vg, const string& name );

	list<string> getCommitActions( bool mark_destructive );
        int commit();

	void setCallbackProgressBar( CallbackProgressBar pfnc )
	    { progress_bar_cb=pfnc; }
	CallbackProgressBar getCallbackProgressBar() { return progress_bar_cb; }
	void setCallbackShowInstallInfo( CallbackShowInstallInfo pfnc )
	    { install_info_cb=pfnc; }
	CallbackShowInstallInfo getCallbackShowInstallInfo() { return install_info_cb; }
	void progressBarCb( const string& id, unsigned cur, unsigned max );
	void showInfoCb( const string& info );

// iterators over container
    protected:
	// protected typedefs for iterators over container
	template< class Pred >
	    struct ConstContainerPI { typedef ContainerIter<Pred, CCIter> type; };
	typedef CheckFnc<const Container> CheckFncCont;
	typedef CheckerIterator< CheckFncCont, ConstContainerPI<CheckFncCont>::type,
	                         CCIter, Container > ConstContPIterator;
	template< class Pred >
	    struct ContainerPI { typedef ContainerIter<Pred, CIter> type; };
	template< class Pred >
	    struct ContainerI
		{ typedef ContainerDerIter<Pred, typename ContainerPI<Pred>::type,
					   Container> type; };
	typedef CheckerIterator< CheckFncCont, ContainerPI<CheckFncCont>::type,
	                         CIter, Container > ContPIterator;
	typedef DerefIterator<ContPIterator,Container> ContIterator;
	typedef IterPair<ContIterator> CPair;

    public:
	// public typedefs for iterators over containers
	template< class Pred >
	    struct ConstContainerI
		{ typedef ContainerDerIter<Pred, typename ConstContainerPI<Pred>::type,
					   const Container> type; };
	template< class Pred >
	    struct ContCondIPair { typedef MakeCondIterPair<Pred,
	                           typename ConstContainerI<Pred>::type> type;};
	typedef DerefIterator<ConstContPIterator,const Container> ConstContIterator;
	typedef IterPair<ConstContIterator> ConstContPair;

	// public member functions for iterators over containers
	ConstContPair contPair( bool (* CheckFnc)( const Container& )=NULL ) const
	    {
	    return( ConstContPair( contBegin( CheckFnc ), contEnd( CheckFnc ) ));
	    }
	ConstContIterator contBegin( bool (* CheckFnc)( const Container& )=NULL ) const
	    {
	    return( ConstContIterator( ConstContPIterator( cont.begin(), cont.end(), CheckFnc )) );
	    }
	ConstContIterator contEnd( bool (* CheckFnc)( const Container& )=NULL ) const
	    {
	    return( ConstContIterator( ConstContPIterator( cont.begin(), cont.end(), CheckFnc, true )) );
	    }
	template< class Pred > typename ContCondIPair<Pred>::type contCondPair( const Pred& p ) const
	    {
	    return( typename ContCondIPair<Pred>::type( contCondBegin( p ), contCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstContainerI<Pred>::type contCondBegin( const Pred& p ) const
	    {
	    return( typename ConstContainerI<Pred>::type( typename ConstContainerPI<Pred>::type( cont.begin(), cont.end(), p )) );
	    }
	template< class Pred > typename ConstContainerI<Pred>::type contCondEnd( const Pred& p ) const
	    {
	    return( typename ConstContainerI<Pred>::type( typename ConstContainerPI<Pred>::type( cont.begin(), cont.end(), p, true )) );
	    }
    protected:
	// protected member functions for iterators over containers
	CPair cPair( bool (* CheckFnc)( const Container& )=NULL )
	    {
	    return( CPair( cBegin(CheckFnc), cEnd(CheckFnc) ));
	    }
	ContIterator cBegin( bool (* CheckFnc)( const Container& )=NULL )
	    {
	    return( ContIterator( ContPIterator( cont.begin(), cont.end(), CheckFnc )) );
	    }
	ContIterator cEnd( bool (* CheckFnc)( const Container& )=NULL )
	    {
	    return( ContIterator( ContPIterator( cont.begin(), cont.end(), CheckFnc, true )) );
	    }

// iterators over disks
    protected:
	// protected typedefs for iterators over disks
	typedef CastCheckIterator<CCIter, DISK, const Disk *> ContainerCDiskIter;
	template< class Pred >
	    struct ConstDiskPI { typedef ContainerIter<Pred, ContainerCDiskIter> type; };
	typedef CastCheckIterator<CIter, DISK, Disk *> ContainerDiskIter;
	template< class Pred >
	    struct DiskPI { typedef ContainerIter<Pred, ContainerDiskIter> type; };
	template< class Pred >
	    struct DiskI { typedef ContainerDerIter<Pred, typename DiskPI<Pred>::type, Disk> type; };
	typedef CheckFnc<const Disk> CheckFncDisk;
	typedef CheckerIterator< CheckFncDisk, ConstDiskPI<CheckFncDisk>::type,
	                         ContainerCDiskIter, Disk > ConstDiskPIterator;
	typedef CheckerIterator< CheckFncDisk, DiskPI<CheckFncDisk>::type,
	                         ContainerDiskIter, Disk > DiskPIterator;
	typedef DerefIterator<DiskPIterator,Disk> DiskIterator;
	typedef IterPair<DiskIterator> DiskPair;

    public:
	// public typedefs for iterators over disks
	typedef DerefIterator<ConstDiskPIterator,const Disk> ConstDiskIterator;
	template< class Pred >
	    struct ConstDiskI
		{ typedef ContainerDerIter<Pred, typename ConstDiskPI<Pred>::type,
					   const Disk> type; };
	template< class Pred >
	    struct DiskCondIPair { typedef MakeCondIterPair<Pred, typename ConstDiskI<Pred>::type> type; };
	typedef IterPair<ConstDiskIterator> ConstDiskPair;

	// public member functions for iterators over disks
	ConstDiskPair diskPair( bool (* CheckFnc)( const Disk& )=NULL ) const
	    {
	    return( ConstDiskPair( diskBegin( CheckFnc ), diskEnd( CheckFnc ) ));
	    }
	ConstDiskIterator diskBegin( bool (* CheckFnc)( const Disk& )=NULL ) const
	    {
	    IterPair<ContainerCDiskIter> p( ContainerCDiskIter( cont.begin(), cont.end() ),
	                                    ContainerCDiskIter( cont.begin(), cont.end(), true ));
	    return( ConstDiskIterator( ConstDiskPIterator( p, CheckFnc )) );
	    }
	ConstDiskIterator diskEnd( bool (* CheckFnc)( const Disk& )=NULL ) const
	    {
	    IterPair<ContainerCDiskIter> p( ContainerCDiskIter( cont.begin(), cont.end() ),
	                                    ContainerCDiskIter( cont.begin(), cont.end(), true ));
	    return( ConstDiskIterator( ConstDiskPIterator( p, CheckFnc, true )) );
	    }
	template< class Pred > typename DiskCondIPair<Pred>::type diskCondPair( const Pred& p ) const
	    {
	    return( typename DiskCondIPair<Pred>::type( diskCondBegin( p ), diskCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstDiskI<Pred>::type diskCondBegin( const Pred& p ) const
	    {
	    IterPair<ContainerCDiskIter> pair( ContainerCDiskIter( cont.begin(), cont.end() ),
					       ContainerCDiskIter( cont.begin(), cont.end(), true ));
	    return( typename ConstDiskI<Pred>::type( typename ConstDiskPI<Pred>::type( pair, p )) );
	    }
	template< class Pred > typename ConstDiskI<Pred>::type diskCondEnd( const Pred& p ) const
	    {
	    IterPair<ContainerCDiskIter> pair( ContainerCDiskIter( cont.begin(), cont.end() ),
					       ContainerCDiskIter( cont.begin(), cont.end(), true ));
	    return( typename ConstDiskI<Pred>::type( typename ConstDiskPI<Pred>::type( pair, p, true )) );
	    }
    protected:
	// protected member functions for iterators over disks
	DiskPair dPair( bool (* CheckFnc)( const Disk& )=NULL )
	    {
	    return( DiskPair( dBegin( CheckFnc ), dEnd( CheckFnc ) ));
	    }
	DiskIterator dBegin( bool (* CheckFnc)( const Disk& )=NULL )
	    {
	    IterPair<ContainerDiskIter> p( ContainerDiskIter( cont.begin(), cont.end() ),
	                                   ContainerDiskIter( cont.begin(), cont.end(), true ));
	    return( DiskIterator( DiskPIterator( p, CheckFnc )) );
	    }
	DiskIterator dEnd( bool (* CheckFnc)( const Disk& )=NULL )
	    {
	    IterPair<ContainerDiskIter> p( ContainerDiskIter( cont.begin(), cont.end() ),
	                                   ContainerDiskIter( cont.begin(), cont.end(), true ));
	    return( DiskIterator( DiskPIterator( p, CheckFnc, true )) );
	    }


// iterators over LVM VGs
    protected:
	// protected typedefs for iterators over LVM VGs
	typedef CastCheckIterator<CCIter, LVM, const LvmVg *> ContainerCLvmVgIter;
	template< class Pred >
	    struct ConstLvmVgPI { typedef ContainerIter<Pred, ContainerCLvmVgIter> type; };
	typedef CastCheckIterator<CIter, LVM, LvmVg *> ContainerLvmVgIter;
	template< class Pred >
	    struct LvmVgPI { typedef ContainerIter<Pred, ContainerLvmVgIter> type; };
	template< class Pred >
	    struct LvmVgI { typedef ContainerDerIter<Pred, typename LvmVgPI<Pred>::type, LvmVg> type; };
	typedef CheckFnc<const LvmVg> CheckFncLvmVg;
	typedef CheckerIterator< CheckFncLvmVg, ConstLvmVgPI<CheckFncLvmVg>::type,
	                         ContainerCLvmVgIter, LvmVg > ConstLvmVgPIterator;

    public:
	// public typedefs for iterators over LVM VGs
	typedef DerefIterator<ConstLvmVgPIterator,const LvmVg> ConstLvmVgIterator;
	template< class Pred >
	    struct ConstLvmVgI
		{ typedef ContainerDerIter<Pred, typename ConstLvmVgPI<Pred>::type,
					   const LvmVg> type; };
	template< class Pred >
	    struct LvmVgCondIPair { typedef MakeCondIterPair<Pred, typename ConstLvmVgI<Pred>::type> type; };
	typedef IterPair<ConstLvmVgIterator> ConstLvmVgPair;

	// public member functions for iterators over LVM VGs
	ConstLvmVgPair lvmVgPair( bool (* CheckFnc)( const LvmVg& )=NULL ) const
	    {
	    return( ConstLvmVgPair( lvmVgBegin( CheckFnc ), lvmVgEnd( CheckFnc ) ));
	    }
	ConstLvmVgIterator lvmVgBegin( bool (* CheckFnc)( const LvmVg& )=NULL ) const
	    {
	    IterPair<ContainerCLvmVgIter> p( ContainerCLvmVgIter( cont.begin(), cont.end() ),
	                                     ContainerCLvmVgIter( cont.begin(), cont.end(), true ));
	    return( ConstLvmVgIterator( ConstLvmVgPIterator( p, CheckFnc )) );
	    }
	ConstLvmVgIterator lvmVgEnd( bool (* CheckFnc)( const LvmVg& )=NULL ) const
	    {
	    IterPair<ContainerCLvmVgIter> p( ContainerCLvmVgIter( cont.begin(), cont.end() ),
	                                     ContainerCLvmVgIter( cont.begin(), cont.end(), true ));
	    return( ConstLvmVgIterator( ConstLvmVgPIterator( p, CheckFnc, true )) );
	    }
	template< class Pred > typename LvmVgCondIPair<Pred>::type lvmVgCondPair( const Pred& p ) const
	    {
	    return( typename LvmVgCondIPair<Pred>::type( lvmVgCondBegin( p ), lvmVgCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstLvmVgI<Pred>::type lvmVgCondBegin( const Pred& p ) const
	    {
	    IterPair<ContainerCLvmVgIter> pair( ContainerCLvmVgIter( cont.begin(), cont.end() ),
					        ContainerCLvmVgIter( cont.begin(), cont.end(), true ));
	    return( typename ConstLvmVgI<Pred>::type( typename ConstLvmVgPI<Pred>::type( pair, p )) );
	    }
	template< class Pred > typename ConstLvmVgI<Pred>::type lvmVgCondEnd( const Pred& p ) const
	    {
	    IterPair<ContainerCLvmVgIter> pair( ContainerCLvmVgIter( cont.begin(), cont.end() ),
					        ContainerCLvmVgIter( cont.begin(), cont.end(), true ));
	    return( typename ConstLvmVgI<Pred>::type( typename ConstLvmVgPI<Pred>::type( pair, p, true )) );
	    }
    protected:
	// protected member functions for iterators over LVM VGs

// iterators over EVMS container
    protected:
	// protected typedefs for iterators over EVMS container
	typedef CastCheckIterator<CCIter, EVMS, const Evms *> ContainerCEvmsIter;
	template< class Pred >
	    struct ConstEvmsPI { typedef ContainerIter<Pred, ContainerCEvmsIter> type; };
	typedef CastCheckIterator<CIter, EVMS, Evms *> ContainerEvmsIter;
	template< class Pred >
	    struct EvmsPI { typedef ContainerIter<Pred, ContainerEvmsIter> type; };
	template< class Pred >
	    struct EvmsI { typedef ContainerDerIter<Pred, typename EvmsPI<Pred>::type, Evms> type; };
	typedef CheckFnc<const Evms> CheckFncEvms;
	typedef CheckerIterator< CheckFncEvms, ConstEvmsPI<CheckFncEvms>::type,
	                         ContainerCEvmsIter, Evms > ConstEvmsPIterator;

    public:
	// public typedefs for iterators over EVMS container
	typedef DerefIterator<ConstEvmsPIterator,const Evms> ConstEvmsIterator;
	template< class Pred >
	    struct ConstEvmsI
		{ typedef ContainerDerIter<Pred, typename ConstEvmsPI<Pred>::type,
					   const Evms> type; };
	template< class Pred >
	    struct EvmsCondIPair { typedef MakeCondIterPair<Pred, typename ConstEvmsI<Pred>::type> type; };
	typedef IterPair<ConstEvmsIterator> ConstEvmsPair;

	// public member functions for iterators over EVMS container
	ConstEvmsPair evmsPair( bool (* CheckFnc)( const Evms& )=NULL ) const
	    {
	    return( ConstEvmsPair( evmsBegin( CheckFnc ), evmsEnd( CheckFnc ) ));
	    }
	ConstEvmsIterator evmsBegin( bool (* CheckFnc)( const Evms& )=NULL ) const
	    {
	    IterPair<ContainerCEvmsIter> p( ContainerCEvmsIter( cont.begin(), cont.end() ),
	                                    ContainerCEvmsIter( cont.begin(), cont.end(), true ));
	    return( ConstEvmsIterator( ConstEvmsPIterator( p, CheckFnc )) );
	    }
	ConstEvmsIterator evmsEnd( bool (* CheckFnc)( const Evms& )=NULL ) const
	    {
	    IterPair<ContainerCEvmsIter> p( ContainerCEvmsIter( cont.begin(), cont.end() ),
	                                    ContainerCEvmsIter( cont.begin(), cont.end(), true ));
	    return( ConstEvmsIterator( ConstEvmsPIterator( p, CheckFnc, true )) );
	    }
	template< class Pred > typename EvmsCondIPair<Pred>::type evmsCondPair( const Pred& p ) const
	    {
	    return( typename EvmsCondIPair<Pred>::type( evmsCondBegin( p ), evmsCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstEvmsI<Pred>::type evmsCondBegin( const Pred& p ) const
	    {
	    IterPair<ContainerCEvmsIter> pair( ContainerCEvmsIter( cont.begin(), cont.end() ),
					       ContainerCEvmsIter( cont.begin(), cont.end(), true ));
	    return( typename ConstEvmsI<Pred>::type( typename ConstEvmsPI<Pred>::type( pair, p )) );
	    }
	template< class Pred > typename ConstEvmsI<Pred>::type evmsCondEnd( const Pred& p ) const
	    {
	    IterPair<ContainerCEvmsIter> pair( ContainerCEvmsIter( cont.begin(), cont.end() ),
					       ContainerCEvmsIter( cont.begin(), cont.end(), true ));
	    return( typename ConstEvmsI<Pred>::type( typename ConstEvmsPI<Pred>::type( pair, p, true )) );
	    }
    protected:
	// protected member functions for iterators over EVMS container

// iterators over volumes
    protected:
	// protected typedefs for iterators over volumes
	typedef ListListIterator<Container::ConstPlainIterator, ConstContIterator> ConstVolInter;
	template< class Pred >
	    struct ConstVolumePI { typedef ContainerIter<Pred, ConstVolInter> type; };
	typedef CheckFnc<const Volume> CheckFncVol;
	typedef CheckerIterator< CheckFncVol, ConstVolumePI<CheckFncVol>::type,
	                         ConstVolInter, Volume > ConstVolPIterator;
	typedef ListListIterator<Container::PlainIterator, ContIterator> VolPart;
	template< class Pred >
	    struct VolumeI { typedef ContainerIter<Pred, VolPart> type; };
	typedef CheckerIterator< CheckFncVol, VolumeI<CheckFncVol>::type,
	                         VolPart, Volume > VolPIterator;
	typedef DerefIterator<VolPIterator,Volume> VolIterator;
	typedef IterPair<VolIterator> VPair;

    public:
	// public typedefs for iterators over volumes
	template< class Pred >
	    struct ConstVolumeI { typedef ContainerDerIter<Pred, typename ConstVolumePI<Pred>::type, const Volume> type; };
	template< class Pred >
	    struct VolCondIPair { typedef MakeCondIterPair<Pred, typename ConstVolumeI<Pred>::type> type;};
	typedef DerefIterator<ConstVolPIterator,const Volume> ConstVolIterator;
	typedef IterPair<ConstVolIterator> ConstVolPair;

	// public member functions for iterators over volumes
	ConstVolPair volPair( bool (* CheckCnt)( const Container& )) const
	    {
	    return( ConstVolPair( volBegin( CheckCnt ), volEnd( CheckCnt ) ));
	    }
	ConstVolPair volPair( bool (* CheckVol)( const Volume& )=NULL,
			      bool (* CheckCnt)( const Container& )=NULL) const
	    {
	    return( ConstVolPair( volBegin( CheckVol, CheckCnt ),
	                          volEnd( CheckVol, CheckCnt ) ));
	    }
	ConstVolIterator volBegin( bool (* CheckCnt)( const Container& )) const
	    {
	    return( volBegin( NULL, CheckCnt ) );
	    }
	ConstVolIterator volBegin( bool (* CheckVol)( const Volume& )=NULL,
	                           bool (* CheckCnt)( const Container& )=NULL) const
	    {
	    IterPair<ConstVolInter> p( (ConstVolInter( contPair( CheckCnt ))),
				       (ConstVolInter( contPair( CheckCnt ), true )));
	    return( ConstVolIterator( ConstVolPIterator(p, CheckVol )));
	    }
	ConstVolIterator volEnd( bool (* CheckCnt)( const Container& )) const
	    {
	    return( volEnd( NULL, CheckCnt ) );
	    }
	ConstVolIterator volEnd( bool (* CheckVol)( const Volume& )=NULL,
	                         bool (* CheckCnt)( const Container& )=NULL) const
	    {
	    IterPair<ConstVolInter> p( (ConstVolInter( contPair( CheckCnt ))),
				       (ConstVolInter( contPair( CheckCnt ), true )));
	    return( ConstVolIterator( ConstVolPIterator(p, CheckVol, true )));
	    }
	template< class Pred > typename VolCondIPair<Pred>::type volCondPair( const Pred& p ) const
	    {
	    return( typename VolCondIPair<Pred>::type( volCondBegin( p ), volCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstVolumeI<Pred>::type volCondBegin( const Pred& p ) const
	    {
	    IterPair<ConstVolInter> pair( (ConstVolInter( contPair())),
					  (ConstVolInter( contPair(), true )));
	    return( typename ConstVolumeI<Pred>::type( typename ConstVolumePI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstVolumeI<Pred>::type volCondEnd( const Pred& p ) const
	    {
	    IterPair<ConstVolInter> pair( (ConstVolInter( contPair())),
					  (ConstVolInter( contPair(), true )));
	    return( typename ConstVolumeI<Pred>::type( typename ConstVolumePI<Pred>::type(pair, p, true )) );
	    }

    protected:
	// protected member functions for iterators over volumes
	VPair vPair( bool (* CheckCnt)( const Container& ))
	    {
	    return( VPair( vBegin( CheckCnt ), vEnd( CheckCnt ) ));
	    }
	VPair vPair( bool (* CheckVol)( const Volume& )=NULL,
		     bool (* CheckCnt)( const Container& )=NULL)
	    {
	    return( VPair( vBegin( CheckVol, CheckCnt ),
			   vEnd( CheckVol, CheckCnt ) ));
	    }
	VolIterator vBegin( bool (* CheckCnt)( const Container& ))
	    {
	    return( vBegin( NULL, CheckCnt ) );
	    }
	VolIterator vBegin( bool (* CheckVol)( const Volume& )=NULL,
			    bool (* CheckCnt)( const Container& )=NULL)
	    {
	    IterPair<VolPart> p( (VolPart( cPair( CheckCnt ))),
				 (VolPart( cPair( CheckCnt ), true )));
	    return( VolIterator( VolPIterator( p, CheckVol )));
	    }
	VolIterator vEnd( bool (* CheckCnt)( const Container& ))
	    {
	    return( vEnd( NULL, CheckCnt ) );
	    }
	VolIterator vEnd( bool (* CheckVol)( const Volume& )=NULL,
			  bool (* CheckCnt)( const Container& )=NULL)
	    {
	    IterPair<VolPart> p( (VolPart( cPair( CheckCnt ))),
				 (VolPart( cPair( CheckCnt ), true )));
	    return( VolIterator( VolPIterator( p, CheckVol, true )));
	    }

// iterators over partitions
    protected:
	// protected typedefs for iterators over partitions
	typedef ListListIterator<Container::ConstPlainIterator, ConstDiskIterator> ConstPartInter;
	typedef CastIterator<ConstPartInter, Partition *> ConstPartInter2;
	template< class Pred >
	    struct ConstPartitionPI { typedef ContainerIter<Pred, ConstPartInter2> type; };
	typedef CheckFnc<const Partition> CheckFncPartition;
	typedef CheckerIterator< CheckFncPartition, ConstPartitionPI<CheckFncPartition>::type,
	                         ConstPartInter2, Partition > ConstPartPIterator;
    public:
	// public typedefs for iterators over partitions
	template< class Pred >
	    struct ConstPartitionI
		{ typedef ContainerDerIter<Pred, typename ConstPartitionPI<Pred>::type,
		                           const Partition> type; };
	template< class Pred >
	    struct PartCondIPair
		{ typedef MakeCondIterPair<Pred, typename ConstPartitionI<Pred>::type> type;};
	typedef DerefIterator<ConstPartPIterator, const Partition> ConstPartIterator;
	typedef IterPair<ConstPartIterator> ConstPartPair;

	// public member functions for iterators over partitions
	ConstPartPair partPair( bool (* CheckCnt)( const Disk& )) const
	    {
	    return( ConstPartPair( partBegin( CheckCnt ), partEnd( CheckCnt ) ));
	    }
	ConstPartPair partPair( bool (* CheckPart)( const Partition& )=NULL,
				bool (* CheckCnt)( const Disk& )=NULL) const
	    {
	    return( ConstPartPair( partBegin( CheckPart, CheckCnt ),
	                           partEnd( CheckPart, CheckCnt ) ));
	    }
	ConstPartIterator partBegin( bool (* CheckDisk)( const Disk& )) const
	    {
	    return( partBegin( NULL, CheckDisk ) );
	    }
	ConstPartIterator partBegin( bool (* CheckPart)( const Partition& )=NULL,
	                             bool (* CheckDisk)( const Disk& )=NULL) const
	    {
	    IterPair<ConstPartInter2> p( (ConstPartInter(diskPair( CheckDisk ))),
					 (ConstPartInter(diskPair( CheckDisk ), true )));
	    return( ConstPartIterator( ConstPartPIterator(p, CheckPart )));
	    }
	ConstPartIterator partEnd( bool (* CheckDisk)( const Disk& )) const
	    {
	    return( partEnd( NULL, CheckDisk ) );
	    }
	ConstPartIterator partEnd( bool (* CheckPart)( const Partition& )=NULL,
	                           bool (* CheckDisk)( const Disk& )=NULL) const
	    {
	    IterPair<ConstPartInter2> p( (ConstPartInter(diskPair( CheckDisk ))),
					 (ConstPartInter(diskPair( CheckDisk ), true )));
	    return( ConstPartIterator( ConstPartPIterator(p, CheckPart, true )));
	    }
	template< class Pred > typename PartCondIPair<Pred>::type partCondPair( const Pred& p ) const
	    {
	    return( typename PartCondIPair<Pred>::type( partCondBegin( p ), partCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstPartitionI<Pred>::type partCondBegin( const Pred& p ) const
	    {
	    IterPair<ConstPartInter2> pair( (ConstPartInter( diskPair())),
					    (ConstPartInter( diskPair(), true )));
	    return( typename ConstPartitionI<Pred>::type( typename ConstPartitionPI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstPartitionI<Pred>::type partCondEnd( const Pred& p ) const
	    {
	    IterPair<ConstPartInter2> pair( (ConstPartInter( diskPair())),
					    (ConstPartInter( diskPair(), true )));
	    return( typename ConstPartitionI<Pred>::type( typename ConstPartitionPI<Pred>::type(pair, p, true )) );
	    }

// iterators over LVM LVs
    protected:
	// protected typedefs for iterators over LVM LVs
	typedef ListListIterator<Container::ConstPlainIterator, ConstLvmVgIterator> ConstLvmLvInter;
	typedef CastIterator<ConstLvmLvInter, LvmLv *> ConstLvmLvInter2;
	template< class Pred >
	    struct ConstLvmLvPI { typedef ContainerIter<Pred, ConstLvmLvInter2> type; };
	typedef CheckFnc<const LvmLv> CheckFncLvmLv;
	typedef CheckerIterator< CheckFncLvmLv, ConstLvmLvPI<CheckFncLvmLv>::type,
	                         ConstLvmLvInter2, LvmLv > ConstLvmLvPIterator;
    public:
	// public typedefs for iterators over LVM LVs
	template< class Pred >
	    struct ConstLvmLvI
		{ typedef ContainerDerIter<Pred, typename ConstLvmLvPI<Pred>::type,
		                           const LvmLv> type; };
	template< class Pred >
	    struct LvmLvCondIPair
		{ typedef MakeCondIterPair<Pred, typename ConstLvmLvI<Pred>::type> type;};
	typedef DerefIterator<ConstLvmLvPIterator, const LvmLv> ConstLvmLvIterator;
	typedef IterPair<ConstLvmLvIterator> ConstLvmLvPair;

	// public member functions for iterators over LVM LVs
	ConstLvmLvPair lvmLvPair( bool (* CheckLvmVg)( const LvmVg& )) const
	    {
	    return( ConstLvmLvPair( lvmLvBegin( CheckLvmVg ), lvmLvEnd( CheckLvmVg ) ));
	    }
	ConstLvmLvPair lvmLvPair( bool (* CheckLvmLv)( const LvmLv& )=NULL,
				  bool (* CheckLvmVg)( const LvmVg& )=NULL) const
	    {
	    return( ConstLvmLvPair( lvmLvBegin( CheckLvmLv, CheckLvmVg ),
	                            lvmLvEnd( CheckLvmLv, CheckLvmVg ) ));
	    }
	ConstLvmLvIterator lvmLvBegin( bool (* CheckLvmVg)( const LvmVg& )) const
	    {
	    return( lvmLvBegin( NULL, CheckLvmVg ) );
	    }
	ConstLvmLvIterator lvmLvBegin( bool (* CheckLvmLv)( const LvmLv& )=NULL,
	                               bool (* CheckLvmVg)( const LvmVg& )=NULL) const
	    {
	    IterPair<ConstLvmLvInter2> p( (ConstLvmLvInter(lvmVgPair( CheckLvmVg ))),
					  (ConstLvmLvInter(lvmVgPair( CheckLvmVg ), true )));
	    return( ConstLvmLvIterator( ConstLvmLvPIterator(p, CheckLvmLv )));
	    }
	ConstLvmLvIterator lvmLvEnd( bool (* CheckLvmVg)( const LvmVg& )) const
	    {
	    return( lvmLvEnd( NULL, CheckLvmVg ) );
	    }
	ConstLvmLvIterator lvmLvEnd( bool (* CheckLvmLv)( const LvmLv& )=NULL,
	                             bool (* CheckLvmVg)( const LvmVg& )=NULL) const
	    {
	    IterPair<ConstLvmLvInter2> p( (ConstLvmLvInter(lvmVgPair( CheckLvmVg ))),
					  (ConstLvmLvInter(lvmVgPair( CheckLvmVg ), true )));
	    return( ConstLvmLvIterator( ConstLvmLvPIterator(p, CheckLvmLv, true )));
	    }
	template< class Pred > typename LvmLvCondIPair<Pred>::type lvmLvCondPair( const Pred& p ) const
	    {
	    return( typename LvmLvCondIPair<Pred>::type( lvmLvCondBegin( p ), lvmLvCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstLvmLvI<Pred>::type lvmLvCondBegin( const Pred& p ) const
	    {
	    IterPair<ConstLvmLvInter2> pair( (ConstLvmLvInter( lvmVgPair())),
					     (ConstLvmLvInter( lvmVgPair(), true )));
	    return( typename ConstLvmLvI<Pred>::type( typename ConstLvmLvPI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstLvmLvI<Pred>::type lvmLvCondEnd( const Pred& p ) const
	    {
	    IterPair<ConstLvmLvInter2> pair( (ConstLvmLvInter( lvmVgPair())),
					     (ConstLvmLvInter( lvmVgPair(), true )));
	    return( typename ConstLvmLvI<Pred>::type( typename ConstLvmLvPI<Pred>::type(pair, p, true )) );
	    }

// iterators over EVMS volumes
    protected:
	// protected typedefs for iterators over EVMS volumes
	typedef ListListIterator<Container::ConstPlainIterator, ConstEvmsIterator> ConstEvmsVolInter;
	typedef CastIterator<ConstEvmsVolInter, EvmsVol *> ConstEvmsVolInter2;
	template< class Pred >
	    struct ConstEvmsVolPI { typedef ContainerIter<Pred, ConstEvmsVolInter2> type; };
	typedef CheckFnc<const EvmsVol> CheckFncEvmsVol;
	typedef CheckerIterator< CheckFncEvmsVol, ConstEvmsVolPI<CheckFncEvmsVol>::type,
	                         ConstEvmsVolInter2, EvmsVol > ConstEvmsVolPIterator;
    public:
	// public typedefs for iterators over EVMS volumes
	template< class Pred >
	    struct ConstEvmsVolI
		{ typedef ContainerDerIter<Pred, typename ConstEvmsVolPI<Pred>::type,
		                           const EvmsVol> type; };
	template< class Pred >
	    struct EvmsVolCondIPair
		{ typedef MakeCondIterPair<Pred, typename ConstEvmsVolI<Pred>::type> type;};
	typedef DerefIterator<ConstEvmsVolPIterator, const EvmsVol> ConstEvmsVolIterator;
	typedef IterPair<ConstEvmsVolIterator> ConstEvmsVolPair;

	// public member functions for iterators over EVMS volumes
	ConstEvmsVolPair evmsVolPair( bool (* CheckEvms)( const Evms& )) const
	    {
	    return( ConstEvmsVolPair( evmsVolBegin( CheckEvms ), evmsVolEnd( CheckEvms ) ));
	    }
	ConstEvmsVolPair evmsVolPair( bool (* CheckEvmsVol)( const EvmsVol& )=NULL,
				      bool (* CheckEvms)( const Evms& )=NULL) const
	    {
	    return( ConstEvmsVolPair( evmsVolBegin( CheckEvmsVol, CheckEvms ),
	                              evmsVolEnd( CheckEvmsVol, CheckEvms ) ));
	    }
	ConstEvmsVolIterator evmsVolBegin( bool (* CheckEvms)( const Evms& )) const
	    {
	    return( evmsVolBegin( NULL, CheckEvms ) );
	    }
	ConstEvmsVolIterator evmsVolBegin( bool (* CheckEvmsVol)( const EvmsVol& )=NULL,
	                                   bool (* CheckEvms)( const Evms& )=NULL) const
	    {
	    IterPair<ConstEvmsVolInter2> p( (ConstEvmsVolInter(evmsPair( CheckEvms ))),
					    (ConstEvmsVolInter(evmsPair( CheckEvms ), true )));
	    return( ConstEvmsVolIterator( ConstEvmsVolPIterator(p, CheckEvmsVol )));
	    }
	ConstEvmsVolIterator evmsVolEnd( bool (* CheckEvms)( const Evms& )) const
	    {
	    return( evmsVolEnd( NULL, CheckEvms ) );
	    }
	ConstEvmsVolIterator evmsVolEnd( bool (* CheckEvmsVol)( const EvmsVol& )=NULL,
	                                 bool (* CheckEvms)( const Evms& )=NULL) const
	    {
	    IterPair<ConstEvmsVolInter2> p( (ConstEvmsVolInter(evmsPair( CheckEvms ))),
					    (ConstEvmsVolInter(evmsPair( CheckEvms ), true )));
	    return( ConstEvmsVolIterator( ConstEvmsVolPIterator(p, CheckEvmsVol, true )));
	    }
	template< class Pred > typename EvmsVolCondIPair<Pred>::type evmsVolCondPair( const Pred& p ) const
	    {
	    return( typename EvmsVolCondIPair<Pred>::type( evmsVolCondBegin( p ), evmsVolCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstEvmsVolI<Pred>::type evmsVolCondBegin( const Pred& p ) const
	    {
	    IterPair<ConstEvmsVolInter2> pair( (ConstEvmsVolInter( evmsPair())),
					       (ConstEvmsVolInter( evmsPair(), true )));
	    return( typename ConstEvmsVolI<Pred>::type( typename ConstEvmsVolPI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstEvmsVolI<Pred>::type evmsVolCondEnd( const Pred& p ) const
	    {
	    IterPair<ConstEvmsVolInter2> pair( (ConstEvmsVolInter( evmsPair())),
					       (ConstEvmsVolInter( evmsPair(), true )));
	    return( typename ConstEvmsVolI<Pred>::type( typename ConstEvmsVolPI<Pred>::type(pair, p, true )) );
	    }

// iterators over software raid devices
    protected:
	// protected typedefs for iterators over software raid devices
	typedef CastIterator<ConstVolInter, Md *> ConstMdInter;
	template< class Pred >
	    struct ConstMdPI { typedef ContainerIter<Pred,
	                                             ConstMdInter> type; };
	typedef CheckFnc<const Md> CheckFncMd;
	typedef CheckerIterator< CheckFncMd, ConstMdPI<CheckFncMd>::type,
	                         ConstMdInter, Md > ConstMdPIterator;
    public:
	// public typedefs for iterators over software raid devices
	template< class Pred >
	    struct ConstMdI
		{ typedef ContainerDerIter<Pred, typename ConstMdPI<Pred>::type,
		                           const Md> type; };
	template< class Pred >
	    struct MdCondIPair
		{ typedef MakeCondIterPair<Pred, typename ConstMdI<Pred>::type> type;};
	typedef DerefIterator<ConstMdPIterator, const Md> ConstMdIterator;
	typedef IterPair<ConstMdIterator> ConstMdPair;

	// public member functions for iterators over software raid devices
	ConstMdPair mdPair( bool (* CheckMd)( const Md& )=NULL ) const
	    {
	    return( ConstMdPair( mdBegin( CheckMd ), mdEnd( CheckMd ) ));
	    }
	ConstMdIterator mdBegin( bool (* CheckMd)( const Md& )=NULL ) const
	    {
	    ConstVolInter b( contPair( isMd ) );
	    ConstVolInter e( contPair( isMd ), true );
	    IterPair<ConstMdInter> p( (ConstMdInter(b)), (ConstMdInter(e)) );
	    return( ConstMdIterator( ConstMdPIterator(p, CheckMd )));
	    }
	ConstMdIterator mdEnd( bool (* CheckMd)( const Md& )=NULL ) const
	    {
	    ConstVolInter b( contPair( isMd ) );
	    ConstVolInter e( contPair( isMd ), true );
	    IterPair<ConstMdInter> p( (ConstMdInter(b)), (ConstMdInter(e)) );
	    return( ConstMdIterator( ConstMdPIterator(p, CheckMd, true )));
	    }
	template< class Pred > typename MdCondIPair<Pred>::type mdCondPair( const Pred& p ) const
	    {
	    return( typename MdCondIPair<Pred>::type( mdCondBegin( p ), mdCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstMdI<Pred>::type mdCondBegin( const Pred& p ) const
	    {
	    ConstVolInter b( contPair( isMd ) );
	    ConstVolInter e( contPair( isMd ), true );
	    IterPair<ConstMdInter> pair( (ConstMdInter(b)), (ConstMdInter(e)) );
	    return( typename ConstMdI<Pred>::type( typename ConstMdPI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstMdI<Pred>::type mdCondEnd( const Pred& p ) const
	    {
	    ConstVolInter b( contPair( isMd ) );
	    ConstVolInter e( contPair( isMd ), true );
	    IterPair<ConstMdInter> pair( (ConstMdInter(b)), (ConstMdInter(e)) );
	    return( typename ConstMdI<Pred>::type( typename ConstMdPI<Pred>::type(pair, p, true )) );
	    }

// iterators over file based loop devices
    protected:
	// protected typedefs for iterators over file based loop devices
	typedef CastIterator<ConstVolInter, Loop *> ConstLoopInter;
	template< class Pred >
	    struct ConstLoopPI { typedef ContainerIter<Pred,
	                                             ConstLoopInter> type; };
	typedef CheckFnc<const Loop> CheckFncLoop;
	typedef CheckerIterator< CheckFncLoop, ConstLoopPI<CheckFncLoop>::type,
	                         ConstLoopInter, Loop > ConstLoopPIterator;
    public:
	// public typedefs for iterators over file based loop devices
	template< class Pred >
	    struct ConstLoopI
		{ typedef ContainerDerIter<Pred, typename ConstLoopPI<Pred>::type,
		                           const Loop> type; };
	template< class Pred >
	    struct LoopCondIPair
		{ typedef MakeCondIterPair<Pred, typename ConstLoopI<Pred>::type> type;};
	typedef DerefIterator<ConstLoopPIterator, const Loop> ConstLoopIterator;
	typedef IterPair<ConstLoopIterator> ConstLoopPair;

	// public member functions for iterators over file based loop devices
	ConstLoopPair loopPair( bool (* CheckLoop)( const Loop& )=NULL ) const
	    {
	    return( ConstLoopPair( loopBegin( CheckLoop ), loopEnd( CheckLoop ) ));
	    }
	ConstLoopIterator loopBegin( bool (* CheckLoop)( const Loop& )=NULL ) const
	    {
	    ConstVolInter b( contPair( isLoop ) );
	    ConstVolInter e( contPair( isLoop ), true );
	    IterPair<ConstLoopInter> p( (ConstLoopInter(b)), (ConstLoopInter(e)) );
	    return( ConstLoopIterator( ConstLoopPIterator(p, CheckLoop )));
	    }
	ConstLoopIterator loopEnd( bool (* CheckLoop)( const Loop& )=NULL ) const
	    {
	    ConstVolInter b( contPair( isLoop ) );
	    ConstVolInter e( contPair( isLoop ), true );
	    IterPair<ConstLoopInter> p( (ConstLoopInter(b)), (ConstLoopInter(e)) );
	    return( ConstLoopIterator( ConstLoopPIterator(p, CheckLoop, true )));
	    }
	template< class Pred > typename LoopCondIPair<Pred>::type loopCondPair( const Pred& p ) const
	    {
	    return( typename LoopCondIPair<Pred>::type( loopCondBegin( p ), loopCondEnd( p ) ) );
	    }
	template< class Pred > typename ConstLoopI<Pred>::type loopCondBegin( const Pred& p ) const
	    {
	    ConstVolInter b( contPair( isLoop ) );
	    ConstVolInter e( contPair( isLoop ), true );
	    IterPair<ConstLoopInter> pair( (ConstLoopInter(b)), (ConstLoopInter(e)) );
	    return( typename ConstLoopI<Pred>::type( typename ConstLoopPI<Pred>::type(pair, p) ) );
	    }
	template< class Pred > typename ConstLoopI<Pred>::type loopCondEnd( const Pred& p ) const
	    {
	    ConstVolInter b( contPair( isLoop ) );
	    ConstVolInter e( contPair( isLoop ), true );
	    IterPair<ConstLoopInter> pair( (ConstLoopInter(b)), (ConstLoopInter(e)) );
	    return( typename ConstLoopI<Pred>::type( typename ConstLoopPI<Pred>::type(pair, p, true )) );
	    }

    protected:
	// protected internal member functions
	void initialize();
	void detectDisks();
	void detectLvmVgs();
	void autodetectDisks();
	void detectFsData( const VolIterator& begin, const VolIterator& end );
	void detectFsDataTestMode( const string& file, 
	                           const VolIterator& begin, const VolIterator& end );
	static void detectArch();
	void addToList( Container* e )
	    { pointerIntoSortedList<Container>( cont, e ); }
	DiskIterator findDisk( const string& disk );
	bool findVolume( const string& device, ContIterator& c, VolIterator& v  );
	bool findVolume( const string& device, VolIterator& v  );
	void logVolumes( const string& Dir );

	// protected internal member variables
	bool readonly;
	bool testmode;
	bool inst_sys;
	bool cache;
	bool initialized;
	bool autodetect;
	string testdir;
	string tempdir;
	string rootprefix;
	string logdir;
	static string proc_arch;
	CCont cont;
	EtcFstab *fstab;
	CallbackProgressBar progress_bar_cb;
	CallbackShowInstallInfo install_info_cb;
	unsigned max_log_num;
    };

#endif
