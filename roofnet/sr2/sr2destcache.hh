#ifndef CLICK_SR2DESTCACHE_HH
#define CLICK_SR2DESTCACHE_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include <click/etheraddress.hh>
#include <click/vector.hh>
#include <click/hashmap.hh>
#include <click/deque.hh>
#include <elements/wifi/linktable.hh>
#include <elements/ethernet/arptable.hh>
#include <elements/wifi/path.hh>
#include <click/ipflowid.hh>
#include <clicknet/tcp.h>
#include "sr2packet.hh"
#include "sr2gatewayselector.hh"
CLICK_DECLS

/*
 * =c
 * SR2DestCache([GW ipaddress], [SEL GatewaySelector element])
 * =s Roofnet
 * =d
 *
 * This element marks the gateway for a packet to be sent to.
 * Either manually specifiy an gw using the GW keyword
 * or automatically select it using a GatewaySelector element.
 * 
 *
 */


class SR2DestCache : public Element {
 public:
  
  SR2DestCache();
  ~SR2DestCache();
  
  const char *class_name() const		{ return "SR2DestCache"; }
  const char *port_count() const		{ return "2/2"; }
  const char *processing() const		{ return PUSH; }
  const char *flow_code() const			{ return "#/#"; }


  /* handler stuff */
  void add_handlers();
  static String read_param(Element *e, void *vparam);
  void push(int, Packet *);
private:

  class CacheEntry {
  public:
    IPAddress _client;
    IPAddress _ap;
    Timestamp _last;
    CacheEntry() {
	    _last = Timestamp::now();
    }

    CacheEntry(const CacheEntry &e) : 
	    _client(e._client),
	    _ap(e._ap),
	    _last(e._last)
	    { }
    CacheEntry(IPAddress c, IPAddress ap) {
	    CacheEntry();
	    _client = c;
	    _ap = ap;
    }

  };

  typedef HashMap<IPAddress, CacheEntry> Cache;
  typedef Cache::const_iterator FTIter;
  Cache _cache;

};


CLICK_ENDDECLS
#endif
