// bittorrent.h
//

#ifndef BITTORRENT_H
#define BITTORRENT_H

#include <stdint.h>

#include "bencode.h"
#include "datum.h"
#include "json_object.h"
#include "lex.h"
#include "match.h"
#include "newhttp.h"


class bittorrent_dht {
    bencoding::dictionary dict;

public:
    bittorrent_dht (datum &d) : dict(d) { }

    void write_json(struct json_object &o, bool) {
        if (this->is_not_empty()) {
            struct json_object dht{o, "bittorrent_dht"};
            dict.write_json(dht);
            dht.close();
        }
    }

    bool is_not_empty() { return dict.is_not_empty(); }

    static constexpr mask_and_value<8> matcher {
        {0xff, 0xff, 0xff, 0x8c, 0xff, 0xff, 0xff, 0xff},
        {'d', '1', ':', 0x00, 'd', '2', ':', 'i'}
    };
};
      
// Local Service Discovery (LSD) uses the following multicast groups:
// A) 239.192.152.143:6771 (org-local) and B) [ff15::efc0:988f]:6771
// (site-local)
//
// Implementation note: Since the multicast groups have a broader
// scope than lan-local implementations may want to set the
// IP_MULTICAST_TTL socket option to a value above 1
//
//   An LSD announce is formatted as follows:
//
//   BT-SEARCH * HTTP/1.1\r\n
//   Host: <host>\r\n
//   Port: <port>\r\n
//   Infohash: <ihash>\r\n
//   cookie: <cookie (optional)>\r\n
//   \r\n
//   \r\n
//
// host: RFC 2616 section 14.23 and RFC 2732 compliant Host header
// specifying the multicast group to which the announce is sent. In
// other words, strings A) or B), as appropriate.
//
// port: port on which the bittorrent client is listening in base-10,
// ascii
//
// ihash: hex-encoded (40 character) infohash.  An announce may
// contain multiple, consecutive Infohash headers to announce the
// participation in more than one torrent. This may not be supported
// by older implementations. When sending multiple infohashes the
// packet length should not exceed 1400 bytes to avoid
// MTU/fragmentation problems.
//
// cookie: opaque value, allowing the sending client to filter out its
// own announces if it receives them via multicast loopback

class header {
    newhttp::token name;
    ignore_char_class<space> sp;
    newhttp::text value;

public:

    header(datum &d) :
            name{d},
            sp{d},
            value{d}
        { }

    void write_json(json_array &a) {
        if (is_not_empty()) {
            json_object hdr{a};
            hdr.print_key_json_string("key", name);
            hdr.print_key_json_string("value", value);
            hdr.close();
        }
    }

    bool is_not_empty() const { return value.is_not_empty(); }
};

class lsd_header : public datum {
public:
    lsd_header(datum &d) : datum{d} { }

    void write_json(json_object &o) const {
        json_array hdrs{o, "headers"};
        datum tmp{*this};
        while (tmp.is_not_empty()) {
            if (lookahead<newhttp::crlf> at_end{tmp}) {
                break;
            }

            header h{tmp};
            if (!h.is_not_empty()) {
                break;
            }

            h.write_json(hdrs);
            newhttp::crlf ignore{tmp};
        }
        hdrs.close();
    }
};

class bittorrent_lsd {
    literal_byte<'B', 'T', '-', 'S', 'E', 'A', 'R', 'C', 'H'> proto;
    ignore_char_class<space> sp1;
    literal_byte<'*'> asterisk;
    ignore_char_class<space> sp2;
    newhttp::version version;
    newhttp::crlf crlf;
    lsd_header headers;
    bool valid; 

public:

    bittorrent_lsd(datum &d) :
        proto{d},
        sp1(d),
        asterisk{d},
        sp2(d),
        version(d),
        crlf(d),
        headers{d},
        valid{d.is_not_null()} { }

    bool is_not_empty() { return valid; }

    void write_json(struct json_object &o, bool) {
        if (this->is_not_empty()) {
            struct json_object lsd{o, "bittorrent_lsd"};
            lsd.print_key_json_string("version", version);
            headers.write_json(lsd);
            lsd.close();
        }
    }

    static constexpr mask_and_value<8> matcher {
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        {'B', 'T', '-', 'S', 'E', 'A', 'R', 'C'}
    };
        
};

class bittorrent_peer_message {
    encoded<uint32_t> message_length;
    encoded<uint8_t> message_type;
    datum message;
    bool valid;
    
public:
    
    bittorrent_peer_message(datum &d) :
        message_length{d},
        message_type{d},
        message{d, message_length - 1},
        valid{d.is_not_null()}
    {
    }

    const char* get_code_str() const {
        switch(message_type.value()) {
        case 0x00:          return "choke";
        case 0x01:          return "unchoke";
        case 0x02:          return "interested";
        case 0x03:          return "not_interested";
        case 0x04:          return "have";
        case 0x05:          return "bit_field";
        case 0x06:          return "request";
        case 0x07:          return "piece";
        case 0x08:          return "cancel";
        case 0x14:          return "extended";
        default:            return nullptr;
        }
    }

    uint8_t get_code() const {return message_type.value();}
    
    void write_json(struct json_array &o) {
        if(!valid) {
            return;
        }

        struct json_object msg{o};
        msg.print_key_uint("message_length", message_length);
        type_codes<bittorrent_peer_message> code{*this};
        msg.print_key_value("mesage_type", code);
        msg.print_key_hex("message", message);
        msg.close();
    }
};

//
// The peer wire protocol consists of a handshake followed by a
// never-ending stream of length-prefixed messages. The handshake
// starts with character ninteen (decimal) followed by the string
// 'BitTorrent protocol'. The leading character is a length prefix,
// put there in the hope that other new protocols may do the same and
// thus be trivially distinguishable from each other.
// All later integers sent in the protocol are encoded as four bytes
// big-endian.
//
// After the fixed headers come eight reserved bytes, which are all
// zero in all current implementations. If you wish to extend the
// protocol using these bytes, please coordinate with Bram Cohen to
// make sure all extensions are done compatibly.
//
// Next comes the 20 byte sha1 hash of the bencoded form of the info
// value from the metainfo file. (This is the same value which is
// announced as info_hash to the tracker, only here it's raw instead
// of quoted here). If both sides don't send the same value, they
// sever the connection. The one possible exception is if a downloader
// wants to do multiple downloads over a single port, they may wait
// for incoming connections to give a download hash first, and respond
// with the same one if it's in their list.
//
// After the download hash comes the 20-byte peer id which is reported
// in tracker requests and contained in peer lists in tracker
// responses. If the receiving side's peer id doesn't match the one
// the initiating side expects, it severs the connection.
//
// That's it for handshaking, next comes an alternating stream of
// length prefixes and messages. Messages of length zero are
// keepalives, and ignored. Keepalives are generally sent once every
// two minutes, but note that timeouts can be done much more quickly
// when data is expected.

class bittorrent_handshake {
    literal_byte<0x13, 'B', 'i', 't', 'T', 'o', 'r', 'r', 'e', 'n', 't', ' ', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l'> protocol;
    datum extension_bytes;
    datum hash_of_info_dict;
    datum peer_id;
    datum body;
    bool valid;

public:
    bittorrent_handshake(datum &d) :
        protocol{d},
        extension_bytes{d, 8},
        hash_of_info_dict{d, 20},
        peer_id{d, 20},
        body{d},
        valid{d.is_not_null()} { }

    bool is_not_empty() const { return valid; }

    static constexpr mask_and_value<8> matcher {
        {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        {0x13, 'B', 'i', 't', 'T', 'o', 'r', 'r'}
    };

    void write_json(struct json_object &o, bool) {
        if (this->is_not_empty()) {
            struct json_object bt{o, "bittorrent"};
            bt.print_key_hex("extension_bytes", extension_bytes);
            bt.print_key_hex("info_dict", hash_of_info_dict);
            bt.print_key_hex("peer_id", peer_id);

            struct json_array msgs{bt, "messages"};
            while(body.is_not_empty()) {
                bittorrent_peer_message peer_msg{body};
                peer_msg.write_json(msgs);
            }
            msgs.close();
            bt.close();
        }
    }    

    void fprint(FILE *f) const {
        fprintf(f, "extension_bytes:   ");  extension_bytes.fprint_hex(f);   fputc('\n', f);
        fprintf(f, "hash_of_info_dict: ");  hash_of_info_dict.fprint_hex(f); fputc('\n', f);
        fprintf(f, "peer_id:           ");  peer_id.fprint_hex(f);           fputc('\n', f);
    }
};

[[maybe_unused]] inline static int bittorrent_dht_fuzz_test(const uint8_t *data, size_t size) {
    struct datum request_data{data, data+size};
    char buffer[8192];
    struct buffer_stream buf_json(buffer, sizeof(buffer));
    struct json_object record(&buf_json);

    bittorrent_dht dht{request_data};
    if (dht.is_not_empty()) {
        dht.write_json(record, true);
    }

    return 0;
}

[[maybe_unused]] inline static int bittorrent_lsd_fuzz_test(const uint8_t *data, size_t size) {
    struct datum request_data{data, data+size};
    char buffer[8192];
    struct buffer_stream buf_json(buffer, sizeof(buffer));
    struct json_object record(&buf_json);

    bittorrent_lsd lsd{request_data};
    if (lsd.is_not_empty()) {
        lsd.write_json(record, true);
    }

    return 0;
}

[[maybe_unused]] inline static int bittorrent_handshake_fuzz_test(const uint8_t *data, size_t size) {
    struct datum request_data{data, data+size};
    char buffer[8192];
    struct buffer_stream buf_json(buffer, sizeof(buffer));
    struct json_object record(&buf_json);

    bittorrent_handshake handshake{request_data};
    if (handshake.is_not_empty()) {
        handshake.write_json(record, true);
    }

    return 0;
}
#endif // BITTORRENT_H
