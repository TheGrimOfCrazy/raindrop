#include "GameGlobal.h"
#include "Song.h"
#include "SongList.h"

#include "Song7K.h"
#include "SongDC.h"
#include "SongLoader.h"

SongList::SongList (SongList* Parent)
	: mParent(Parent)
{
}

SongList::~SongList()
{
}

void SongList::AddSong(shared_ptr<Game::Song> Song)
{
	ListEntry NewEntry;
	NewEntry.Kind = ListEntry::Song;
	NewEntry.Data = Song;

	mChildren.push_back(NewEntry);
}

void SongList::AddNamedDirectory(boost::mutex &loadMutex, SongLoader *Loader, Directory Dir, GString Name, bool VSRGActive, bool DotcurActive)
{
	bool EntryWasPushed = false;
	SongList* NewList = new SongList(this);

	ListEntry NewEntry;

	NewEntry.EntryName = Name;
	NewEntry.Kind = ListEntry::Directory;
	NewEntry.Data = shared_ptr<void>(NewList);

	std::vector<VSRG::Song*> Songs7K;
	std::vector<dotcur::Song*> SongsDC;
	std::vector<GString> Listing;

	Dir.ListDirectory(Listing, Directory::FS_DIR);

	for (std::vector<GString>::iterator i = Listing.begin();
		i != Listing.end();
		++i)
	{
		if (*i == "." || *i == "..") continue;

		if (VSRGActive)
			Loader->LoadSong7KFromDir(Dir / *i, Songs7K);

		if (DotcurActive)
			Loader->LoadSongDCFromDir(Dir / *i, SongsDC);

		if (!SongsDC.size() && !Songs7K.size()) // No songs, so, time to recursively search.
		{
			if (!EntryWasPushed)
			{
				boost::mutex::scoped_lock lock(loadMutex);
				mChildren.push_back(NewEntry);
				EntryWasPushed = true;
			}

			NewList->AddDirectory(loadMutex, Loader, Dir / *i, VSRGActive, DotcurActive);


			{
				boost::mutex::scoped_lock lock(loadMutex);
				if (!NewList->GetNumEntries())
				{
					if (mChildren.size())
						mChildren.erase(mChildren.end() - 1);
					EntryWasPushed = false;
				}
			}
		}
		else
		{
			size_t tSize = Songs7K.size() + SongsDC.size();

			if (Songs7K.size())
			{
				boost::mutex::scoped_lock lock(loadMutex);

				for (auto j = Songs7K.begin();
					j != Songs7K.end();
					++j)
				{
					NewList->AddSong( shared_ptr<Game::Song>(*j) );
				}

				Songs7K.clear();

			}

			if (SongsDC.size())
			{
				boost::mutex::scoped_lock lock(loadMutex);

				for (auto j = SongsDC.begin();
					j != SongsDC.end();
					j++)
				{
					NewList->AddSong(shared_ptr<Game::Song>(*j));
				}

				SongsDC.clear();
			}

			if (tSize > 0) // There's a song in here. 
			{
				if (!EntryWasPushed)
				{
					boost::mutex::scoped_lock lock(loadMutex);
					mChildren.push_back(NewEntry);
					EntryWasPushed = true;
				}
			}
		}
	}
}

void SongList::AddDirectory(boost::mutex &loadMutex, SongLoader *Loader, Directory Dir, bool VSRGActive, bool DotcurActive)
{
	int idx = Dir.path().find_last_of('/') + 1;
	GString path;
	if (idx == Dir.path().length() - 1) // it ends with a /
	{
		idx = Dir.ParentDirectory().path().find_last_of('/') + 1;
		Dir.ParentDirectory().path().substr(idx);
	}
	else
		path = Dir.path().substr(idx);

	AddNamedDirectory(loadMutex, Loader, Dir, path, VSRGActive, DotcurActive);
}

void SongList::AddVirtualDirectory(GString NewEntryName, Game::Song* List, int Count)
{
	SongList* NewList = new SongList(this);

	ListEntry NewEntry;
	NewEntry.EntryName = NewEntryName;
	NewEntry.Kind = ListEntry::Directory;
	NewEntry.Data = shared_ptr <void> (NewList);

	for (int i = 0; i < Count; i++)
		NewList->AddSong(shared_ptr<Game::Song> (&List[Count]));

	mChildren.push_back(NewEntry);
}

// if false, it's a song
bool SongList::IsDirectory(unsigned int Entry)
{
	if (Entry >= mChildren.size()) return true;
	return mChildren[Entry].Kind == ListEntry::Directory;
}

shared_ptr<SongList> SongList::GetListEntry(unsigned int Entry)
{
	assert(IsDirectory(Entry));
	return static_pointer_cast<SongList> (mChildren[Entry].Data);
}

shared_ptr<Game::Song> SongList::GetSongEntry(unsigned int Entry)
{
	if (!IsDirectory(Entry))
		return static_pointer_cast<Game::Song> (mChildren[Entry].Data);
	else
		return NULL;
}

GString SongList::GetEntryTitle(unsigned int Entry)
{
	if (Entry >= mChildren.size())
		return "";

	if (mChildren[Entry].Kind == ListEntry::Directory)
		return mChildren[Entry].EntryName;
	else
	{
		shared_ptr<Game::Song> Song = static_pointer_cast<Game::Song>(mChildren[Entry].Data);
		return Song->SongName;
	}
}

unsigned int SongList::GetNumEntries()
{
	return mChildren.size();
}

bool SongList::HasParentDirectory()
{
	return mParent != NULL;
}

SongList* SongList::GetParentDirectory()
{
	return mParent;
}
