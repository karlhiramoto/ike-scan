/*
 * The IKE Scanner (ike-scan) is Copyright (C) 2003 Roy Hills, NTA Monitor Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * If this license is unacceptable to you, I may be willing to negotiate
 * alternative licenses (contact ike-scan@nta-monitor.com).
 *
 * You are encouraged to send comments, improvements or suggestions to
 * me at ike-scan@nta-monitor.com.
 *
 * $Id$
 *
 * Author: Roy Hills
 * Date: 7 November 2003
 *
 * Functions to construct ISAKMP headers and payloads.
 *
 */

#include "ike-scan.h"

static char rcsid[] = "$Id$";	/* RCS ID for ident(1) */

const char *notification_msg[] = { /* Notify Message Types from RFC 2408 3.14.1 */
   "UNSPECIFIED",                    /* 0 */
   "INVALID-PAYLOAD-TYPE",           /* 1 */
   "DOI-NOT-SUPPORTED",              /* 2 */
   "SITUATION-NOT-SUPPORTED",        /* 3 */
   "INVALID-COOKIE",                 /* 4 */
   "INVALID-MAJOR-VERSION",          /* 5 */
   "INVALID-MINOR-VERSION",          /* 6 */
   "INVALID-EXCHANGE-TYPE",          /* 7 */
   "INVALID-FLAGS",                  /* 8 */
   "INVALID-MESSAGE-ID",             /* 9 */
   "INVALID-PROTOCOL-ID",            /* 10 */
   "INVALID-SPI",                    /* 11 */
   "INVALID-TRANSFORM-ID",           /* 12 */
   "ATTRIBUTES-NOT-SUPPORTED",       /* 13 */
   "NO-PROPOSAL-CHOSEN",             /* 14 */
   "BAD-PROPOSAL-SYNTAX",            /* 15 */
   "PAYLOAD-MALFORMED",              /* 16 */
   "INVALID-KEY-INFORMATION",        /* 17 */
   "INVALID-ID-INFORMATION",         /* 18 */
   "INVALID-CERT-ENCODING",          /* 19 */
   "INVALID-CERTIFICATE",            /* 20 */
   "CERT-TYPE-UNSUPPORTED",          /* 21 */
   "INVALID-CERT-AUTHORITY",         /* 22 */
   "INVALID-HASH-INFORMATION",       /* 23 */
   "AUTHENTICATION-FAILED",          /* 24 */
   "INVALID-SIGNATURE",              /* 25 */
   "ADDRESS-NOTIFICATION",           /* 26 */
   "NOTIFY-SA-LIFETIME",             /* 27 */
   "CERTIFICATE-UNAVAILABLE",        /* 28 */
   "UNSUPPORTED-EXCHANGE-TYPE",      /* 29 */
   "UNEQUAL-PAYLOAD-LENGTHS"         /* 30 */
};

/*
 *	make_isakmp_hdr -- Construct an ISAKMP Header
 *
 *	Inputs:
 *
 *	xchg	Exchange Type (e.g. ISAKMP_XCHG_IDPROT for main mode)
 *	next	Next Payload Type
 *	length	ISAKMP Message total length
 *
 *	Returns:
 *
 *	Pointer to created ISAKMP Header.
 *
 *	This constructs an ISAKMP header.  It fills in the static values.
 *	The initator cookie should be changed to a unique per-host value
 *	before the packet is sent.
 */
struct isakmp_hdr*
make_isakmp_hdr(uint8_t xchg, uint8_t next, uint32_t length) {
   struct isakmp_hdr* hdr;

   hdr = Malloc(sizeof(struct isakmp_hdr));
   memset(hdr, '\0', sizeof(struct isakmp_hdr));

   hdr->isa_icookie[0] = 0xdeadbeef;	/* Initiator cookie */
   hdr->isa_icookie[1] = 0xdeadbeef;
   hdr->isa_rcookie[0] = 0;		/* Set responder cookie to 0 */
   hdr->isa_rcookie[1] = 0;
   hdr->isa_np = next;			/* Next Payload Type */
   hdr->isa_version = 0x10;		/* v1.0 */
   hdr->isa_xchg = xchg;		/* Exchange type */
   hdr->isa_flags = 0;			/* No flags */
   hdr->isa_msgid = 0;			/* MBZ for phase-1 */
   hdr->isa_length = htonl(length);	/* Total ISAKMP message length */

   return hdr;
}

/*
 *	make_sa_hdr -- Construct an SA Header
 *
 *	Inputs:
 *
 *	next    Next Payload Type
 *	length	SA payload length
 *
 *	Returns:
 *
 *	Pointer to SA Header.
 *
 *	This constructs an SA header.  It fills in the static values.
 */
struct isakmp_sa*
make_sa_hdr(uint8_t next, uint32_t length) {
   struct isakmp_sa* hdr;

   hdr = Malloc(sizeof(struct isakmp_sa));
   memset(hdr, '\0', sizeof(struct isakmp_sa));

   hdr->isasa_np = next;		/* Next Payload Type */
   hdr->isasa_length = htons(length);		/* SA Payload length */
   hdr->isasa_doi = htonl(ISAKMP_DOI_IPSEC);	/* IPsec DOI */
   hdr->isasa_situation = htonl(SIT_IDENTITY_ONLY);	/* Exchange type */

   return hdr;
}

/*
 *	make_prop -- Construct a proposal payload
 *
 *	Inputs:
 *
 *	length	Proposal payload length
 *	notrans	Number of transforms in this proposal
 *
 *	Returns:
 *
 *	Pointer to proposal payload.
 *
 *	This constructs a proposal payload.  It fills in the static values.
 *	We assume only one proposal will be created.  I think that ISAKMP SAs
 *	are only allowed to have one proposal anyway.
 */
struct isakmp_proposal*
make_prop(uint32_t length, uint8_t notrans) {
   struct isakmp_proposal* hdr;

   hdr = Malloc(sizeof(struct isakmp_proposal));
   memset(hdr, '\0', sizeof(struct isakmp_proposal));

   hdr->isap_np = 0;			/* No more proposals */
   hdr->isap_length = htons(length);	/* Proposal payload length */
   hdr->isap_proposal = 1;		/* Proposal #1 */
   hdr->isap_protoid = PROTO_ISAKMP;
   hdr->isap_spisize = 0;		/* No SPI */
   hdr->isap_notrans = notrans;		/* Number of transforms */

   return hdr;
}

/*
 *	make_trans -- Construct a single transform payload
 *
 *	Inputs:
 *
 *	length	(output) length of entire transform payload.
 *	next    Next Payload Type (3 = More transforms; 0=No more transforms)
 *	number	Transform number
 *	cipher	The encryption algorithm
 *	keylen	Key length for variable length keys (0=fixed key length)
 *	hash	Hash algorithm
 *	auth	Authentication method
 *	group	DH Group number
 *	lifetime	Lifetime in seconds (0=no lifetime)
 *	lifesize	Life in kilobytes (0=no life)
 *
 *	Returns:
 *
 *	Pointer to transform payload.
 *
 *	This constructs a single transform payload.
 *	Most of the values are defined in RFC 2409 Appendix A.
 */
unsigned char*
make_trans(size_t *length, uint8_t next, uint8_t number, uint16_t cipher,
           uint16_t keylen, uint16_t hash, uint16_t auth, uint16_t group,
           uint32_t lifetime, uint32_t lifesize, int gss_id_flag,
           unsigned char *gss_data, size_t gss_data_len) {

   struct isakmp_transform* hdr;	/* Transform header */
   struct isakmp_attribute* attr1;	/* Mandatory attributes */
   struct isakmp_attribute* attr2=NULL;	/* Optional keylen attribute */
   struct isakmp_attribute* attr3=NULL;	/* Optional lifetype attribute */
   struct isakmp_attribute_l32* attr4=NULL; /* Optional lifetime attribute */
   struct isakmp_attribute* attr5=NULL;	/* Optional lifetype attribute */
   struct isakmp_attribute_l32* attr6=NULL; /* Optional lifesize attribute */
   unsigned char *gssid=NULL;		/* Optional GSSID attribute */
   unsigned char *payload;
   unsigned char *cp;
   size_t len;				/* Payload Length */

/* Allocate and initialise the transform header */

   hdr = Malloc(sizeof(struct isakmp_transform));
   memset(hdr, '\0', sizeof(struct isakmp_transform));

   hdr->isat_np = next;			/* Next payload type */
   hdr->isat_transnum = number;		/* Transform Number */
   hdr->isat_transid = KEY_IKE;

/* Allocate and initialise the mandatory attributes */

   attr1 = Malloc(4 * sizeof(struct isakmp_attribute));

   attr1[0].isaat_af_type = htons(0x8001);	/* Encryption Algorithm */
   attr1[0].isaat_lv = htons(cipher);
   attr1[1].isaat_af_type = htons(0x8002);	/* Hash Algorithm */
   attr1[1].isaat_lv = htons(hash);
   attr1[2].isaat_af_type = htons(0x8003);	/* Authentication Method */
   attr1[2].isaat_lv = htons(auth);
   attr1[3].isaat_af_type = htons(0x8004);	/* Group Description */
   attr1[3].isaat_lv = htons(group);

   len = sizeof(struct isakmp_transform) + 4 * sizeof(struct isakmp_attribute);

/* Allocate and initialise the optional attributes */

   if (keylen) {
      attr2 = Malloc(sizeof(struct isakmp_attribute));
      attr2->isaat_af_type = htons(0x800e);	/* Key Length */
      attr2->isaat_lv = htons(keylen);
      len += sizeof(struct isakmp_attribute);
   }

   if (lifetime) {
      attr3 = Malloc(sizeof(struct isakmp_attribute));
      attr4 = Malloc(sizeof(struct isakmp_attribute_l32));
      attr3->isaat_af_type = htons(0x800b);	/* Life Type */
      attr3->isaat_lv = htons(1);		/* Seconds */
      attr4->isaat_af_type = htons(0x000c);	/* Life Duratiion */
      attr4->isaat_l = htons(4);		/* 4 Bytes- CANT CHANGE */
      attr4->isaat_v = htonl(lifetime);		/* Lifetime in seconds */
      len += sizeof(struct isakmp_attribute) +
             sizeof(struct isakmp_attribute_l32);
   }

   if (lifesize) {
      attr5 = Malloc(sizeof(struct isakmp_attribute));
      attr6 = Malloc(sizeof(struct isakmp_attribute_l32));
      attr5->isaat_af_type = htons(0x800b);	/* Life Type */
      attr5->isaat_lv = htons(2);		/* Kilobytes */
      attr6->isaat_af_type = htons(0x000c);	/* Life Duratiion */
      attr6->isaat_l = htons(4);		/* 4 Bytes- CANT CHANGE */
      attr6->isaat_v = htonl(lifesize);		/* Lifetime in seconds */
      len += sizeof(struct isakmp_attribute) +
             sizeof(struct isakmp_attribute_l32);
   }

   if (gss_id_flag) {
      struct isakmp_attribute *gss_hdr;
      gssid = Malloc(gss_data_len + sizeof(struct isakmp_attribute));
      gss_hdr = (struct isakmp_attribute *) gssid;	/* Overlay */
      gss_hdr->isaat_af_type = htons(16384);	/* GSS ID */
      gss_hdr->isaat_lv = htons(gss_data_len);
      memcpy(gssid+sizeof(struct isakmp_attribute), gss_data, gss_data_len);
      len += gss_data_len + sizeof(struct isakmp_attribute);
   }

/* Fill in length value now we know it */

   hdr->isat_length = htons(len);	/* Transform length */
   *length = len;

/* Allocate memory for payload and copy structures to payload */

   payload = Malloc(len);

   cp = payload;
   memcpy(cp, hdr, sizeof(struct isakmp_transform));
   free(hdr);
   cp += sizeof(struct isakmp_transform);
   memcpy(cp, attr1, 4 * sizeof(struct isakmp_attribute));
   free(attr1);
   cp += 4 * sizeof(struct isakmp_attribute);
   if (keylen) {
      memcpy(cp, attr2, sizeof(struct isakmp_attribute));
      free(attr2);
      cp += sizeof(struct isakmp_attribute);
   }
   if (lifetime) {
      memcpy(cp, attr3, sizeof(struct isakmp_attribute));
      free(attr3);
      cp += sizeof(struct isakmp_attribute);
      memcpy(cp, attr4, sizeof(struct isakmp_attribute_l32));
      free(attr4);
      cp += sizeof(struct isakmp_attribute_l32);
   }
   if (lifesize) {
      memcpy(cp, attr5, sizeof(struct isakmp_attribute));
      free(attr5);
      cp += sizeof(struct isakmp_attribute);
      memcpy(cp, attr6, sizeof(struct isakmp_attribute_l32));
      free(attr6);
      cp += sizeof(struct isakmp_attribute_l32);
   }
   if (gss_id_flag) {
      memcpy(cp, gssid, gss_data_len+sizeof(struct isakmp_attribute));
      free(gssid);
      cp += gss_data_len+sizeof(struct isakmp_attribute);
   }


   return payload;
}

/*
 *	add_trans -- Add a transform payload onto the set of transforms.
 *
 *	Inputs:
 *
 *	finished	0 if adding a new transform; 1 if finalising.
 *	length	(output) length of entire transform payload.
 *	cipher	The encryption algorithm
 *	keylen	Key length for variable length keys (0=fixed key length)
 *	hash	Hash algorithm
 *	auth	Authentication method
 *	group	DH Group number
 *	lifetime	Lifetime in seconds (0=no lifetime)
 *
 *	Returns:
 *
 *	Pointer to new set of transform payloads.
 *
 *	This function can either be called with finished = 0, in which case
 *	cipher, keylen, hash, auth, group and lifetime must be specified, and
 *	the function will return NULL, OR it can be called with finished = 1
 *	in which case cipher, keylen, hash, auth, group and lifetime are
 *	ignored and the function will return a pointer to the finished
 *	payload and will set *length to the length of this payload.
 */
unsigned char*
add_trans(int finished, size_t *length,
          uint16_t cipher, uint16_t keylen, uint16_t hash, uint16_t auth,
          uint16_t group, uint32_t lifetime, uint32_t lifesize,
          int gss_id_flag, unsigned char *gss_data, size_t gss_data_len) {

   static int first_transform = 1;
   static unsigned char *trans_start=NULL;	/* Start of set of transforms */
   static size_t cur_offset;			/* Start of current transform */
   static size_t end_offset;			/* End of transforms */
   static int trans_no=1;
   unsigned char *trans;			/* Transform payload */
   size_t len;					/* Transform length */
/*
 * Construct a transform if we are not finalising.
 * Set next to 3 (more transforms), and increment trans_no for next time round.
 */
   if (!finished) {
      trans = make_trans(&len, 3, trans_no, cipher, keylen, hash, auth,
                         group, lifetime, lifesize, gss_id_flag, gss_data,
                         gss_data_len);
      trans_no++;
      if (first_transform) {
         cur_offset = 0;
         end_offset = len;
         trans_start = Malloc(end_offset);
         memcpy(trans_start, trans, len);
         first_transform = 0;
      } else {
         cur_offset = end_offset;
         end_offset += len;
         trans_start = Realloc(trans_start, end_offset);
         memcpy(trans_start+cur_offset, trans, len);
      }
      return NULL;
   } else {
      struct isakmp_transform* hdr =
         (struct isakmp_transform*) (trans_start+cur_offset);	/* Overlay */

      hdr->isat_np = 0;		/* No more transforms */
      *length = end_offset;
      return trans_start;
   }
}

/*
 *	make_vid -- Construct a vendor id payload
 *
 *	Inputs:
 *
 *	length	(output) length of Vendor ID payload.
 *	next		Next Payload Type
 *	vid_data	Vendor ID data
 *	vid_data_len	Vendor ID data length
 *
 *	Returns:
 *
 *	Pointer to vendor id payload.
 *
 *	This constructs a vendor id payload.  It fills in the static values.
 *	The next pointer value must be filled in later.
 */
unsigned char*
make_vid(size_t *length, uint8_t next, unsigned char *vid_data,
         size_t vid_data_len) {
   unsigned char *payload;
   struct isakmp_vid* hdr;

   payload = Malloc(sizeof(struct isakmp_vid)+vid_data_len);
   hdr = (struct isakmp_vid*) payload;	/* Overlay vid struct on payload */
   memset(hdr, '\0', sizeof(struct isakmp_vid));

   hdr->isavid_np = next;		/* Next payload type */
   hdr->isavid_length = htons(sizeof(struct isakmp_vid)+vid_data_len);

   memcpy(payload+sizeof(struct isakmp_vid), vid_data, vid_data_len);
   *length = sizeof(struct isakmp_vid) + vid_data_len;

   return payload;
}

/*
 *	add_vid -- Add a vendor ID payload to the set of VIDs.
 *
 *	Inputs:
 *
 *      finished        0 if adding a new VIDs; 1 if finalising.
 *      length  (output) length of entire VID payload set.
 *      vid_data        Vendor ID data
 *      vid_data_len    Vendor ID data length
 *
 *	Returns:
 *
 *	Pointer to the VID payload.
 *
 *	This function can either be called with finished = 0, in which case
 *	vid_data and vid_data_len must be specified, and
 *	the function will return NULL, OR it can be called with finished = 1
 *	in which case vid_data and vid_data_len are
 *	ignored and the function will return a pointer to the finished
 *	payload and will set *length to the length of this payload.
 */
unsigned char*
add_vid(int finished, size_t *length, unsigned char *vid_data,
        size_t vid_data_len) {
   static int first_vid = 1;
   static unsigned char *vid_start=NULL;	/* Start of set of VIDs */
   static size_t cur_offset;			/* Start of current VID */
   static size_t end_offset;			/* End of VIDs */
   unsigned char *vid;				/* VID payload */
   size_t len;					/* VID length */
/*
 * Construct a VID if we are not finalising.
 */
   if (!finished) {
      vid = make_vid(&len, ISAKMP_NEXT_VID, vid_data, vid_data_len);
      if (first_vid) {
         cur_offset = 0;
         end_offset = len;
         vid_start = Malloc(end_offset);
         memcpy(vid_start, vid, len);
         first_vid = 0;
      } else {
         cur_offset = end_offset;
         end_offset += len;
         vid_start = Realloc(vid_start, end_offset);
         memcpy(vid_start+cur_offset, vid, len);
      }
      return NULL;
   } else {
      struct isakmp_vid* hdr =
         (struct isakmp_vid*) (vid_start+cur_offset);   /* Overlay */

      hdr->isavid_np = ISAKMP_NEXT_NONE;         /* No more payloads */
      *length = end_offset;
      return vid_start;
   }
}

/*
 *	make_ke	-- Make a Key Exchange payload
 *
 *	Inputs:
 *
 *      length		(output) length of key exchange payload.
 *      next		Next Payload Type
 *      kx_data_len	Key exchange data length
 *
 *	Returns:
 *
 *	Pointer to key exchange payload.
 *
 *	A real implementation would fill in the key exchange payload with the
 *	Diffie Hellman public value.  However, we just use random data.
 */
unsigned char*
make_ke(size_t *length, uint8_t next, size_t kx_data_len) {
   unsigned char *payload;
   struct isakmp_kx* hdr;
   unsigned char *kx_data;
   int i;

   if (kx_data_len % 4)
      err_msg("Key exchange data length %d is not a multiple of 4",
              kx_data_len);

   payload = Malloc(sizeof(struct isakmp_kx)+kx_data_len);
   hdr = (struct isakmp_kx*) payload;	/* Overlay kx struct on payload */
   memset(hdr, '\0', sizeof(struct isakmp_kx));

   kx_data = payload + sizeof(struct isakmp_kx);
   for (i=0; i<kx_data_len; i++)
      *(kx_data++) = (unsigned char) (rand() & 0xff);

   hdr->isakx_np = next;		/* Next payload type */
   hdr->isakx_length = htons(sizeof(struct isakmp_kx)+kx_data_len);

   *length = sizeof(struct isakmp_kx) + kx_data_len;

   return payload;
}

/*
 *	make_nonce	-- Make a Nonce payload
 *
 *	Inputs:
 *
 *	length		(output) length of nonce payload.
 *      next		Next Payload Type
 *	nonce_len	Length of nonce data.
 *
 *	Returns:
 *
 *	Pointer to nonce payload.
 *
 *	For a real implementation, the nonce should use strong random numbers.
 *	However, we just use rand() because we don't care about the quality of
 *	the random numbers for this tool.
 */
unsigned char*
make_nonce(size_t *length, uint8_t next, size_t nonce_len) {
   unsigned char *payload;
   struct isakmp_nonce* hdr;
   unsigned char *cp;
   int i;

   payload = Malloc(sizeof(struct isakmp_nonce)+nonce_len);
   hdr = (struct isakmp_nonce*) payload;  /* Overlay nonce struct on payload */
   memset(hdr, '\0', sizeof(struct isakmp_nonce));

   hdr->isanonce_np = next;		/* Next payload type */
   hdr->isanonce_length = htons(sizeof(struct isakmp_nonce)+nonce_len);

   cp = payload+sizeof(struct isakmp_vid);
   for (i=0; i<nonce_len; i++)
      *(cp++) = (unsigned char) (rand() & 0xff);

   *length = sizeof(struct isakmp_nonce)+nonce_len;
   return payload;
}

/*
 *	make_id	-- Make an Identification payload
 *
 *	Inputs:
 *
 *      length		(output) length of ID payload.
 *      next		Next Payload Type
 *	idtype		Identification Type
 *      id_data		ID data
 *      id_data_len	ID data length
 *
 */
unsigned char*
make_id(size_t *length, uint8_t next, uint8_t idtype, unsigned char *id_data,
        size_t id_data_len) {
   unsigned char *payload;
   struct isakmp_id* hdr;

   payload = Malloc(sizeof(struct isakmp_id)+id_data_len);
   hdr = (struct isakmp_id*) payload;	/* Overlay ID struct on payload */
   memset(hdr, '\0', sizeof(struct isakmp_id));

   hdr->isaid_np = next;		/* Next payload type */
   hdr->isaid_length = htons(sizeof(struct isakmp_id)+id_data_len);
   hdr->isaid_idtype = idtype;
   hdr->isaid_doi_specific_a = 17;		/* Protocol: UDP */
   hdr->isaid_doi_specific_b = htons(500);	/* Port: 500 */

   memcpy(payload+sizeof(struct isakmp_id), id_data, id_data_len);
   *length = sizeof(struct isakmp_id) + id_data_len;

   return payload;
}

/*
 *	skip_payload -- Skip an ISAMKP payload
 *
 *	Inputs:
 *
 *	cp	Pointer to start of payload to skip
 *	len	Packet length remaining
 *	next	Next payload type.
 *
 *	Returns:
 *
 *	Pointer to start of next payload, or NULL if no next payload.
 */
unsigned char *
skip_payload(unsigned char *cp, size_t *len, int *next) {
   struct isakmp_generic *hdr = (struct isakmp_generic *) cp;
/*
 *	Signal no more payloads by setting length to zero if:
 *
 *	The packet length is less than the ISAKMP generic header size; or
 *	The payload length is greater than the packet length; or
 *	The payload length is less than the size of the generic header; or
 *	There is no next payload.
 *
 *	Also set *next to none and return null.
 */
   if (*len < sizeof(struct isakmp_generic) ||
       ntohs(hdr->isag_length) >= *len ||
       ntohs(hdr->isag_length) < sizeof(struct isakmp_generic) ||
       hdr->isag_np == ISAKMP_NEXT_NONE) {
      *len=0;
      *next=ISAKMP_NEXT_NONE;
      return NULL;
   }
/*
 *	There is another payload after this one, so adjust length and
 *	return pointer to next payload.
 */
   *len = *len - ntohs(hdr->isag_length);
   *next = hdr->isag_np;
   return cp + ntohs(hdr->isag_length);
}

/*
 *	process_isakmp_hdr -- Process ISAKMP header
 *
 *	Inputs:
 *
 *	cp	Pointer to start of ISAKMP header
 *	len	Packet length remaining
 *	next	Next payload type.
 *	type	Exchange type
 *
 *	Returns:
 *
 *	Pointer to start of next payload, or NULL if no next payload.
 */
unsigned char *
process_isakmp_hdr(unsigned char *cp, size_t *len, int *next, int *type) {
   struct isakmp_hdr *hdr = (struct isakmp_hdr *) cp;
/*
 *	Signal no more payloads by setting length to zero if:
 *
 *	The packet length is less than the ISAKMP header size; or
 *	The payload length is less than the size of the header; or
 *	There is no next payload.
 *
 *	Also set *next to none and return null.
 */
   if (*len < sizeof(struct isakmp_hdr) ||
       ntohl(hdr->isa_length) < sizeof(struct isakmp_hdr) ||
       hdr->isa_np == ISAKMP_NEXT_NONE) {
      *len=0;
      *next=ISAKMP_NEXT_NONE;
      *type=ISAKMP_XCHG_NONE;
      return NULL;
   }
/*
 *	There is another payload after this one, so adjust length and
 *	return pointer to next payload.
 */
   *len = *len - sizeof(struct isakmp_hdr);
   *next = hdr->isa_np;
   *type = hdr->isa_xchg;
   return cp + sizeof(struct isakmp_hdr);
}

/*
 *	process_sa -- Process SA Payload
 *
 *	Inputs:
 *
 *	cp	Pointer to start of SA payload
 *	len	Packet length remaining
 *	type	Exchange type.
 *
 *	Returns:
 *
 *	Pointer to SA description string.
 *
 *	The description string pointer returned points to malloc'ed storage
 *	which should be free'ed by the caller when it's no longer needed.
 */
char *
process_sa(unsigned char *cp, size_t len, int type) {
   struct isakmp_sa *sa_hdr = (struct isakmp_sa *) cp;
   struct isakmp_proposal *prop_hdr =
      (struct isakmp_proposal *) (cp + sizeof(struct isakmp_sa));
   char *msg;
   char *msg2;

   if (len < sizeof(struct isakmp_sa) + sizeof(struct isakmp_proposal) ||
        ntohs(sa_hdr->isasa_length) < sizeof(struct isakmp_sa) +
        sizeof(struct isakmp_proposal))
      return make_message("IKE Handshake returned (packet too short to decode)");

   if (type == ISAKMP_XCHG_IDPROT) {		/* Main Mode */
      msg = make_message("Main Mode Handshake returned");
   } else if (type == ISAKMP_XCHG_AGGR) {	/* Aggressive Mode */
      msg = make_message("Aggressive Mode Handshake returned");
   } else {
      msg = make_message("UNKNOWN Mode Handshake returned (%u)", type);
   }
   if (prop_hdr->isap_notrans != 1) {
      msg2 = msg;
      msg = make_message("%s (%d transforms)", msg2, prop_hdr->isap_notrans);
      free(msg2);
   }
   return msg;
}

/*
 *	process_vid -- Process Vendor ID Payload
 *
 *	Inputs:
 *
 *	cp	Pointer to start of Vendor ID payload
 *	len	Packet length remaining
 *	vidlist	List of Vendor ID patterns.
 *
 *	Returns:
 *
 *	Pointer to Vendor ID description string.
 *
 *	The description string pointer returned points to malloc'ed storage
 *	which should be free'ed by the caller when it's no longer needed.
 */
char *
process_vid(unsigned char *cp, size_t len, struct vid_pattern_list *vidlist) {
   struct isakmp_vid *hdr = (struct isakmp_vid *) cp;
   struct vid_pattern_list *ve;
   char *msg;
   char *p;
   unsigned char *vid_data;
   unsigned char *ucp;
   size_t data_len;
   size_t checklen;
   int i;

   if (len < sizeof(struct isakmp_vid) ||
        ntohs(hdr->isavid_length) < sizeof(struct isakmp_vid))
      return make_message("VID (packet too short to decode)");

   vid_data = cp + sizeof(struct isakmp_vid);  /* Points to start of VID data */
   data_len = ntohs(hdr->isavid_length) < len ? ntohs(hdr->isavid_length) : len;

   msg = Malloc(2*data_len + 1);	/* 2 hex chars/byte + end NULL */
   p = msg;
   ucp=vid_data;
   for (i=0; i<data_len; i++) {
      sprintf(p, "%.2x", (unsigned char) *ucp++);
      p += 2;
   }
   *p = '\0';

   p = msg;
   msg=make_message("VID=%s", p);
   free(p);
/*
 *	Try to find a match in the Vendor ID pattern list.
 */
   ve = vidlist;
   while(ve != NULL) {
      checklen = (data_len<ve->len)?data_len:ve->len;
      if (!(memcmp(vid_data, ve->data, checklen))) {
         p=msg;
         msg=make_message("%s (%s)", p, ve->name);
         free(p);
         break;	/* Stop looking after first match */
      }
      ve=ve->next;
   }

   return msg;
}

/*
 *	process_notify -- Process notify Payload
 *
 *	Inputs:
 *
 *	cp	Pointer to start of notify payload
 *	len	Packet length remaining
 *
 *	Returns:
 *
 *	Pointer to notify description string.
 *
 *	The description string pointer returned points to malloc'ed storage
 *	which should be free'ed by the caller when it's no longer needed.
 */
char *
process_notify(unsigned char *cp, size_t len) {
   struct isakmp_notification *hdr = (struct isakmp_notification *) cp;
   char *msg;
   int msg_type;
   size_t msg_len;
   unsigned char *msg_data;
   unsigned char *notify_msg;

   if (len < sizeof(struct isakmp_notification) ||
        ntohs(hdr->isan_length) < sizeof(struct isakmp_notification))
      return make_message("Notify message (packet too short to decode)");

   msg_type = ntohs(hdr->isan_type);
   msg_len = ntohs(hdr->isan_length) - sizeof(struct isakmp_notification);
   msg_data = cp + sizeof(struct isakmp_notification);

   if (msg_type < 31) {			/* RFC Defined Message Types */
      msg=make_message("Notify message %d (%s)", msg_type,
                       notification_msg[msg_type]);
   } else if (msg_type == 9101) {	/* Firewall-1 4.x/NG Base msg */
      unsigned char *p;
      notify_msg = Malloc(msg_len + 1);	/* Allow extra byte for NULL */
      memcpy(notify_msg, msg_data, msg_len);
      *(notify_msg+msg_len) = '\0';	/* Ensure string is null terminated */
/*
 *      Replace any non-printable characters with "."
 */
      for (p=notify_msg; *p != '\0'; p++) {
         if (!isprint(*p))
            *p='.';
      }
      msg=make_message("Notify message %d [Checkpoint Firewall-1 4.x or NG Base] (%s)",
                       msg_type, notify_msg);
      free(notify_msg);
   } else {
      msg=make_message("Notify message %d (UNKNOWN MESSAGE TYPE)", msg_type);
   }

   return msg;
}

void
isakmp_use_rcsid(void) {
   printf("%s\n", rcsid);	/* Use rcsid to stop compiler optimising away */
}
