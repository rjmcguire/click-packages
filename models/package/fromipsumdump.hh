#ifndef CLICK_FROMIPSUMDUMP_HH
#define CLICK_FROMIPSUMDUMP_HH
#include <click/element.hh>
#include <click/task.hh>

/*
=c

FromIPSummaryDump(FILENAME [, I<KEYWORDS>])

=s sources

reads packets from an IP summary dump file

=d

Reads IP packet descriptors from a file produced by or ToIPSummaryDump, then
creates packets containing info from the descriptors and pushes them out the
output. Optionally stops the driver when there are no more packets.

Keyword arguments are:

=over 8

=item SAMPLE

Unsigned real number between 0 and 1. FromIPSummaryDump will output each
packet with probability SAMPLE. Default is 1. FromIPSummaryDump uses
fixed-point arithmetic, so the actual sampling probability may differ
substantially from the requested sampling probability. Use the
C<sampling_prob> handler to find out the actual probability.

=item STOP

Boolean. If true, then FromIPSummaryDump will ask the router to stop when it
is done reading. Default is false.

=item ACTIVE

Boolean. If false, then FromIPSummaryDump will not emit packets (until the
`C<active>' handler is written). Default is true.

=item ZERO

Boolean. Determines what is in any packet data not set by the dump. If true,
this data is zero. If false (the default), this data is random garbage.

=item PROTO

Byte (0-255). Sets the IP protocol used for output packets when the dump
doesn't specify a protocol. Default is 6 (TCP).

=back

Only available in user-level processes.

=n

Packets generated by FromIPSummaryDump always have IP version 4 and IP header
length 5. The rest of the packet data is zero or garbage, unless set by the
dump. Generated packets will usually have incorrect lengths.

=h sampling_prob read-only

Returns the sampling probability (see the SAMPLE keyword argument).

=h active read/write

Value is a Boolean.

=a

ToIPSummaryDump */

class FromIPSummaryDump : public Element { public:

    FromIPSummaryDump();
    ~FromIPSummaryDump();

    const char *class_name() const	{ return "FromIPSummaryDump"; }
    const char *processing() const	{ return AGNOSTIC; }
    FromIPSummaryDump *clone() const	{ return new FromIPSummaryDump; }

    int configure(const Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void uninitialize();
    void add_handlers();

    void run_scheduled();
    Packet *pull(int);
  
    enum Content {	// must agree with FromIPSummaryDump
	W_NONE, W_TIMESTAMP, W_TIMESTAMP_SEC, W_TIMESTAMP_USEC,
	W_SRC, W_DST, W_LENGTH, W_PROTO, W_IPID, W_SPORT, W_DPORT,
	W_LAST
    };

  private:

    static const uint32_t BUFFER_SIZE = 32768;
    static const int SAMPLING_SHIFT = 28;
    
    int _fd;
    String _buffer;
    int _pos;
    int _len;

    Vector<int> _contents;
    uint16_t _default_proto;
    uint32_t _sampling_prob;
    
    bool _stop : 1;
    bool _format_complaint : 1;
    bool _zero;
    bool _active;

    Task _task;
  
    struct timeval _time_offset;
    String _filename;

    int error_helper(ErrorHandler *, const char *);
    int read_buffer(ErrorHandler *);
    int read_line(String &, ErrorHandler *);

    void bang_data(const String &, ErrorHandler *);
    Packet *read_packet(ErrorHandler *);

    static String read_handler(Element *, void *);
    static int write_handler(const String &, Element *, void *, ErrorHandler *);
    
};

#endif