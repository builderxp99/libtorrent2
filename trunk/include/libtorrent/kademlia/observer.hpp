/*

Copyright (c) 2007, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <boost/pool/pool.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/cstdint.hpp>

namespace libtorrent {
namespace dht {

struct observer;
struct msg;
struct traversal_algorithm;

// defined in rpc_manager.cpp
TORRENT_EXPORT void intrusive_ptr_add_ref(observer const*);
TORRENT_EXPORT void intrusive_ptr_release(observer const*);

// intended struct layout (on 32 bit architectures)
// offset size  alignment field
// 0      8     8         sent
// 8      8     4         m_refs
// 16     4     4         pool_allocator
// 20     16    4         m_addr
// 36     2     2         m_port
// 38     1     1         m_is_v6, m_short_timeout, m_in_constructor, m_was_sent
// 39     1     1         <padding>
// 40

struct observer : boost::noncopyable
{
	friend TORRENT_EXPORT void intrusive_ptr_add_ref(observer const*);
	friend TORRENT_EXPORT void intrusive_ptr_release(observer const*);

	observer(boost::intrusive_ptr<traversal_algorithm> const& a)
		: m_sent()
		, m_refs(0)
		, m_algorithm(a)
		, m_is_v6(false)
		, m_short_timeout(false)
		, m_done(false)
	{
		TORRENT_ASSERT(a);
#ifdef TORRENT_DEBUG
		m_in_constructor = true;
		m_was_sent = false;
#endif
	}

	virtual ~observer();

	// this is called when a reply is received
	virtual void reply(msg const& m) = 0;

	// this is called if no response has been received after
	// a few seconds, before the request has timed out
	void short_timeout();

	bool has_short_timeout() const { return m_short_timeout; }

	// this is called when no reply has been received within
	// some timeout
	void timeout();
	
	// if this is called the destructor should
	// not invoke any new messages, and should
	// only clean up. It means the rpc-manager
	// is being destructed
	void abort();

	ptime sent() const { return m_sent; }

	void set_target(udp::endpoint const& ep);
	address target_addr() const;
	udp::endpoint target_ep() const;

	void set_transaction_id(boost::uint16_t tid)
	{ m_transaction_id = tid; }

	boost::uint16_t transaction_id() const
	{ return m_transaction_id; }

#ifndef TORRENT_DHT_VERBOSE_LOGGING
protected:
#endif

	void done();

	ptime m_sent;

	// reference counter for intrusive_ptr
	mutable boost::detail::atomic_count m_refs;

	const boost::intrusive_ptr<traversal_algorithm> m_algorithm;

	union addr_t
	{
#if TORRENT_USE_IPV6
		address_v6::bytes_type v6;
#endif
		address_v4::bytes_type v4;
	} m_addr;

	boost::uint16_t m_port;

	// the transaction ID for this call
	boost::uint16_t m_transaction_id;

	bool m_is_v6:1;
	bool m_short_timeout:1;
	// when true, this observer has reported
	// back to the traversal algorithm already
	bool m_done:1;

#ifdef TORRENT_DEBUG
public:
	bool m_in_constructor:1;
	bool m_was_sent:1;
#endif
};

typedef boost::intrusive_ptr<observer> observer_ptr;

} }

#endif

