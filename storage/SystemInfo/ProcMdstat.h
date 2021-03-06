/*
 * Copyright (c) [2004-2014] Novell, Inc.
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


#ifndef PROC_MDSTAT_H
#define PROC_MDSTAT_H


#include <map>
#include <vector>


namespace storage
{
    using std::map;
    using std::vector;


    class ProcMdstat
    {
    public:

	ProcMdstat(bool do_probe = true);

	void probe();

	struct Entry
	{
	    Entry() : md_type(RAID_UNK), md_parity(PAR_DEFAULT), size_k(0), chunk_k(0),
		      readonly(false), inactive(false), is_container(false), has_container(false) {}

	    MdType md_type;
	    MdParity md_parity;

	    string super;

	    unsigned long long size_k;
	    unsigned long chunk_k;

	    bool readonly;
	    bool inactive;

	    list<string> devices;
	    list<string> spares;

	    bool is_container;

	    bool has_container;
	    string container_name;
	    string container_member;
	};

	friend std::ostream& operator<<(std::ostream& s, const ProcMdstat& procmdstat);
	friend std::ostream& operator<<(std::ostream& s, const Entry& entry);

	list<string> getEntries() const;

	bool getEntry(const string& name, Entry& entry) const;

	typedef map<string, Entry>::const_iterator const_iterator;

	const_iterator begin() const { return data.begin(); }
	const_iterator end() const { return data.end(); }

	void parse(const vector<string>& lines);

    private:

	Entry parse(const string& line1, const string& line2);

	map<string, Entry> data;

    };


    class MdadmDetail
    {
    public:

	MdadmDetail(const string& device, bool do_probe = true);

	void probe();

	string uuid;
	string devname;
	string metadata;

	friend std::ostream& operator<<(std::ostream& s, const MdadmDetail& mdadmdetail);

	void parse(const vector<string>& lines);

    private:

	string device;

    };


    class MdadmExamine
    {
    public:

	MdadmExamine(const list<string>& devices, bool do_probe = true);

	void probe();

	string metadata;
	string uuid;

	struct Entry
	{
	    Entry() : member(-1) {}

	    int member;
	    string uuid;
	};

	typedef map<string, Entry>::const_iterator const_iterator;

	const_iterator begin() const { return data.begin(); }
	const_iterator end() const { return data.end(); }

	const_iterator find(const string& name) const { return data.find(name); }

	friend std::ostream& operator<<(std::ostream& s, const MdadmExamine& mdadmexamine);
	friend std::ostream& operator<<(std::ostream& s, const Entry& entry);

	void parse(const vector<string>& lines);

    private:

	list<string> devices;

	map<string, Entry> data;

    };

}


#endif
