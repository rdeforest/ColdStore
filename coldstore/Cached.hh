// Cached Mixin for searchable objects
// Copyright Norman Nunley, 2001
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms
// 	$Id

#ifndef __CACHED_MIXIN_H
#define __CACHED_MIXIN_H

template <class Collection,class Cache=VectorSet>
class Cached: public Collection {
public:
	Cached() : Collection(),
		cache(new Cache())
	{
	}
	
	Cached(const Slot& initial) :
		Collection(initial) {
	}

public:
	virtual Slot search(const Slot& symbol) {
		Slot retval = cache->search(symbol);
		if (retval->truth()) {
			return retval;
		}
		retval = this->Collection::search(symbol);
		cache->insert(symbol,retval);
		return retval;
	}

	virtual Slot del(const Slot& symbol) {
		Slot retval = this->Collection::del(symbol);
		invalidate_symbol(symbol);
		return retval;
	}
	

	virtual void invalidate_cache() {
		cache = new Cache();
	}

	virtual Slot invalidate_symbol(Slot symbol) {
		return cache->del(symbol);
	} 

private:
	Slot cache;
};
#endif // __CACHED_MIXIN_H
