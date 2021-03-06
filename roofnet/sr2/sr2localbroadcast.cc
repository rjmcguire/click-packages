/*
 * SR2LocalBroadcast.{cc,hh} -- DSR implementation
 * John Bicket
 *
 * Copyright (c) 1999-2001 Massachussr2localbroadcasts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "sr2localbroadcast.hh"
#include <click/ipaddress.hh>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/straccum.hh>
#include <clicknet/ether.h>
#include "sr2packet.hh"
CLICK_DECLS

SR2LocalBroadcast::SR2LocalBroadcast()
  :  _timer(this), 
     _en(),
     _et(0),
     _packets_originated(0),
     _packets_tx(0),
     _packets_rx(0)
{

  // Pick a starting sequence number that we have not used before.
  _seq = Timestamp::now().usec();

  static unsigned char bcast_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  _bcast = EtherAddress(bcast_addr);
}

SR2LocalBroadcast::~SR2LocalBroadcast()
{
}

int
SR2LocalBroadcast::configure (Vector<String> &conf, ErrorHandler *errh)
{
  int ret;
  _debug = false;
  ret = cp_va_kparse(conf, this, errh,
		     "ETHTYPE", 0, cpUnsignedShort, &_et,
		     "IP", 0, cpIPAddress, &_ip,
		     "BCAST_IP", 0, cpIPAddress, &_bcast_ip,
		     "ETH", 0, cpEtherAddress, &_en,
		     /* below not required */
		     "DEBUG", 0, cpBool, &_debug,
		     cpEnd);

  if (!_et) 
    return errh->error("ETHTYPE not specified");
  if (!_ip) 
    return errh->error("IP not specified");
  if (!_bcast_ip) 
    return errh->error("BCAST_IP not specified");
  if (!_en) 
    return errh->error("ETH not specified");

  return ret;
}

int
SR2LocalBroadcast::initialize (ErrorHandler *)
{
  _timer.initialize (this);
  _timer.schedule_now ();

  return 0;
}

void
SR2LocalBroadcast::run_timer (Timer *)
{
  _timer.schedule_after_msec(1000);
}

void
SR2LocalBroadcast::push(int port, Packet *p_in)
{
  
  if (port == 1) {
    /* from me */
    int hops = 0;
    int extra = sr2packet::len_wo_data(hops) + sizeof(click_ether);
    int payload_len = p_in->length();
    WritablePacket *p = p_in->push(extra);
    if(p == 0)
      return;

    click_ether *eh = (click_ether *) p->data();
    eh->ether_type = htons(_et);
    memcpy(eh->ether_shost, _en.data(), 6);
    memset(eh->ether_dhost, 0xff, 6);

    struct sr2packet *pk = (struct sr2packet *) (eh+1);

    memset(pk, '\0', sr2packet::len_wo_data(hops));
    pk->_version = _sr2_version;
    pk->_type = SR2_PT_DATA;
    pk->set_data_len(payload_len);
    pk->unset_flag(~0);
    pk->set_qdst(_bcast_ip);
    pk->set_seq(++_seq);
    pk->set_num_links(hops);
    pk->set_link_node(0,_ip);
    pk->set_next(0);

    _packets_tx++;
    _packets_originated++;

    output(0).push(p);

  } else {
    _packets_rx++;
    output(1).push(p_in);
  }

}


String
SR2LocalBroadcast::static_print_stats(Element *f, void *)
{
  SR2LocalBroadcast *d = (SR2LocalBroadcast *) f;
  return d->print_stats();
}

String
SR2LocalBroadcast::print_stats()
{
  StringAccum sa;

  sa << "originated " << _packets_originated;
  sa << " tx " << _packets_tx;
  sa << " rx " << _packets_rx;
  sa << "\n";
  return sa.take_string();
}

int
SR2LocalBroadcast::static_write_debug(const String &arg, Element *e,
			void *, ErrorHandler *errh) 
{
  SR2LocalBroadcast *n = (SR2LocalBroadcast *) e;
  bool b;

  if (!cp_bool(arg, &b))
    return errh->error("`debug' must be a boolean");

  n->_debug = b;
  return 0;
}
String
SR2LocalBroadcast::static_print_debug(Element *f, void *)
{
  StringAccum sa;
  SR2LocalBroadcast *d = (SR2LocalBroadcast *) f;
  sa << d->_debug << "\n";
  return sa.take_string();
}
void
SR2LocalBroadcast::add_handlers()
{
  add_read_handler("stats", static_print_stats, 0);
  add_read_handler("debug", static_print_debug, 0);

  add_write_handler("debug", static_write_debug, 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SR2LocalBroadcast)
