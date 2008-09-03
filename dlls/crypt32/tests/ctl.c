/*
 * crypt32 CTL functions tests
 *
 * Copyright 2008 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <wincrypt.h>

#include "wine/test.h"

static const BYTE emptyCTL[] = {
0x30,0x17,0x30,0x00,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x02,0x06,0x00 };
static const BYTE ctlWithOneEntry[] = {
0x30,0x2a,0x30,0x00,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x02,0x06,0x00,0x30,0x11,0x30,0x0f,0x04,
0x01,0x01,0x31,0x0a,0x30,0x08,0x06,0x02,0x2a,0x03,0x31,0x02,0x30,0x00 };
static const BYTE signedCTL[] = {
0x30,0x81,0xc7,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xb9,0x30,0x81,0xb6,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x28,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x1b,0x04,0x19,0x30,0x17,0x30,0x00,
0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,
0x30,0x5a,0x30,0x02,0x06,0x00,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,
0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,
0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,
0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,
0x00,0x04,0x40,0xca,0xd8,0x32,0xd1,0xbd,0x97,0x61,0x54,0xd6,0x80,0xcf,0x0d,
0xbd,0xa2,0x42,0xc7,0xca,0x37,0x91,0x7d,0x9d,0xac,0x8c,0xdf,0x05,0x8a,0x39,
0xc6,0x07,0xc1,0x37,0xe6,0xb9,0xd1,0x0d,0x26,0xec,0xa5,0xb0,0x8a,0x51,0x26,
0x2b,0x4f,0x73,0x44,0x86,0x83,0x5e,0x2b,0x6e,0xcc,0xf8,0x1b,0x85,0x53,0xe9,
0x7a,0x80,0x8f,0x6b,0x42,0x19,0x93 };
static const BYTE signedCTLWithSubjectAlgorithm[] = {
0x30,0x81,0xd1,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xc3,0x30,0x81,0xc0,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x32,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x25,0x04,0x23,0x30,0x21,0x30,0x00,
0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,
0x30,0x5a,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,
0x00,0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,
0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0xca,0x7a,
0xfa,0xbf,0x44,0x36,0xfd,0xb2,0x84,0x70,0x0f,0xf5,0x66,0xcb,0x68,0xdb,0x45,
0x74,0xa1,0xed,0xe3,0x30,0x4a,0x11,0xd2,0x7b,0xf5,0xa9,0x68,0x8d,0x36,0xe8,
0x79,0xff,0xa2,0xf2,0x4c,0x8a,0xa9,0x65,0x03,0xf8,0x77,0xa5,0x01,0xd3,0x46,
0x8a,0xcc,0x93,0x36,0x30,0xe1,0xa4,0x47,0x70,0x3d,0xb3,0x97,0xfc,0x6d,0x24,
0xe9,0xf9 };
static const BYTE signedCTLWithCTLInnerContent[] = {
0x30,0x82,0x01,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x00,0x30,0x81,0xfd,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,
0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x30,0x06,0x09,
0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0xa0,0x23,0x30,0x21,0x30,0x00,
0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,
0x30,0x5a,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,
0x00,0x31,0x81,0xb5,0x30,0x81,0xb2,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0xa0,0x3b,0x30,0x18,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x09,0x03,0x31,0x0b,0x06,0x09,0x2b,0x06,0x01,0x04,
0x01,0x82,0x37,0x0a,0x01,0x30,0x1f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x09,0x04,0x31,0x12,0x04,0x10,0x54,0x71,0xbc,0xe1,0x56,0x31,0xa2,0xf9,
0x65,0x70,0x34,0xf8,0xe2,0xe9,0xb4,0xf4,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x40,0x2f,0x1b,0x9f,0x5a,0x4a,0x15,0x73,0xfa,0xb1,0x93,0x3d,0x09,0x52,0xdf,
0x6b,0x98,0x4b,0x13,0x5e,0xe7,0xbf,0x65,0xf4,0x9c,0xc2,0xb1,0x77,0x09,0xb1,
0x66,0x4d,0x72,0x0d,0xb1,0x1a,0x50,0x20,0xe0,0x57,0xa2,0x39,0xc7,0xcd,0x7f,
0x8e,0xe7,0x5f,0x76,0x2b,0xd1,0x6a,0x82,0xb3,0x30,0x25,0x61,0xf6,0x25,0x23,
0x57,0x6c,0x0b,0x47,0xb8 };
static const BYTE signedCTLWithCTLInnerContentAndBadSig[] = {
0x30,0x82,0x01,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x00,0x30,0x81,0xfd,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,
0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x30,0x06,0x09,
0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0xa0,0x23,0x30,0x21,0x30,0x00,
0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,
0x30,0x5a,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,
0x00,0x31,0x81,0xb5,0x30,0x81,0xb2,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0xa0,0x3b,0x30,0x18,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x09,0x03,0x31,0x0b,0x06,0x09,0x2b,0x06,0x01,0x04,
0x01,0x82,0x37,0x0a,0x01,0x30,0x1f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x09,0x04,0x31,0x12,0x04,0x10,0x54,0x71,0xbc,0xe1,0x56,0x31,0xa2,0xf9,
0x65,0x70,0x34,0xf8,0xe2,0xe9,0xb4,0xf4,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x40,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff };

static void testCreateCTL(void)
{
    PCCTL_CONTEXT ctl;

    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(0, NULL, 0);
    ok(!ctl && GetLastError() == E_INVALIDARG,
     "expected E_INVALIDARG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(X509_ASN_ENCODING, NULL, 0);
    ok(!ctl &&
     (GetLastError() == ERROR_INVALID_DATA ||
      GetLastError() == OSS_MORE_INPUT), /* win9x */
     "expected ERROR_INVALID_DATA, got %d (0x%08x)\n", GetLastError(),
     GetLastError());
    /* An empty CTL can't be created.. */
    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(X509_ASN_ENCODING, emptyCTL, sizeof(emptyCTL));
    ok(!ctl &&
     (GetLastError() == ERROR_INVALID_DATA ||
      GetLastError() == OSS_DATA_ERROR), /* win9x */
     "expected ERROR_INVALID_DATA, got %d (0x%08x)\n", GetLastError(),
     GetLastError());
    /* Nor can any of these "signed" CTLs whose inner content OID isn't
     * szOID_CTL.
     */
    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(X509_ASN_ENCODING, signedCTL, sizeof(signedCTL));
    ok(!ctl &&
     (GetLastError() == ERROR_INVALID_DATA ||
      GetLastError() == CRYPT_E_UNEXPECTED_MSG_TYPE), /* win9x */
     "expected ERROR_INVALID_DATA, got %d (0x%08x)\n", GetLastError(),
     GetLastError());
    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(X509_ASN_ENCODING, ctlWithOneEntry,
     sizeof(ctlWithOneEntry));
    ok(!ctl &&
     (GetLastError() == ERROR_INVALID_DATA ||
      GetLastError() == OSS_DATA_ERROR), /* win9x */
     "expected ERROR_INVALID_DATA, got %d (0x%08x)\n", GetLastError(),
     GetLastError());
    SetLastError(0xdeadbeef);
    ctl = CertCreateCTLContext(X509_ASN_ENCODING,
     signedCTLWithSubjectAlgorithm, sizeof(signedCTLWithSubjectAlgorithm));
    ok(!ctl &&
     (GetLastError() == ERROR_INVALID_DATA ||
      GetLastError() == CRYPT_E_UNEXPECTED_MSG_TYPE), /* win9x */
     "expected ERROR_INVALID_DATA, got %d (0x%08x)\n", GetLastError(),
     GetLastError());
    /* This signed CTL with the appropriate inner content type can be decoded.
     */
    ctl = CertCreateCTLContext(X509_ASN_ENCODING,
     signedCTLWithCTLInnerContent, sizeof(signedCTLWithCTLInnerContent));
    ok(ctl != NULL, "CertCreateCTLContext failed: %08x\n", GetLastError());
    if (ctl)
    {
        /* Even though the CTL was decoded with X509_ASN_ENCODING, the
         * message encoding type is included in the CTL's encoding type.
         */
        ok(ctl->dwMsgAndCertEncodingType ==
         (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
         "expected X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, got %08x\n",
         ctl->dwMsgAndCertEncodingType);
        CertFreeCTLContext(ctl);
    }
    /* This CTL with a bad signature can also be decoded, so the sig isn't
     * checked when loading the CTL.
     */
    ctl = CertCreateCTLContext(X509_ASN_ENCODING,
     signedCTLWithCTLInnerContentAndBadSig,
     sizeof(signedCTLWithCTLInnerContentAndBadSig));
    ok(ctl != NULL, "CertCreateCTLContext failed: %08x\n", GetLastError());
    if (ctl)
        CertFreeCTLContext(ctl);
}

static void checkHash(const BYTE *data, DWORD dataLen, ALG_ID algID,
 PCCTL_CONTEXT context, DWORD propID)
{
    BYTE hash[20] = { 0 }, hashProperty[20];
    BOOL ret;
    DWORD size;

    memset(hash, 0, sizeof(hash));
    memset(hashProperty, 0, sizeof(hashProperty));
    size = sizeof(hash);
    ret = CryptHashCertificate(0, algID, 0, data, dataLen, hash, &size);
    ret = CertGetCTLContextProperty(context, propID, hashProperty, &size);
    ok(ret, "CertGetCTLContextProperty failed: %08x\n", GetLastError());
    if (ret)
        ok(!memcmp(hash, hashProperty, size),
         "Unexpected hash for property %d\n", propID);
}

static void testCTLProperties(void)
{
    PCCTL_CONTEXT ctl;
    BOOL ret;
    DWORD propID, numProps, access, size;

    ctl = CertCreateCTLContext(X509_ASN_ENCODING,
     signedCTLWithCTLInnerContent, sizeof(signedCTLWithCTLInnerContent));
    if (!ctl)
    {
        skip("CertCreateCTLContext failed: %08x\n", GetLastError());
        return;
    }

    /* No properties as yet */
    propID = 0;
    numProps = 0;
    do {
        propID = CertEnumCTLContextProperties(ctl, propID);
        if (propID)
            numProps++;
    } while (propID != 0);
    ok(numProps == 0, "Expected 0 properties, got %d\n", numProps);

    /* An implicit property */
    ret = CertGetCTLContextProperty(ctl, CERT_ACCESS_STATE_PROP_ID, NULL,
     &size);
    ok(ret, "CertGetCTLContextProperty failed: %08x\n", GetLastError());
    ret = CertGetCTLContextProperty(ctl, CERT_ACCESS_STATE_PROP_ID, &access,
     &size);
    ok(ret, "CertGetCTLContextProperty failed: %08x", GetLastError());
    ok(!(access & CERT_ACCESS_STATE_WRITE_PERSIST_FLAG),
     "Didn't expect a persisted cert\n");

    checkHash(signedCTLWithCTLInnerContent,
     sizeof(signedCTLWithCTLInnerContent), CALG_SHA1, ctl, CERT_HASH_PROP_ID);

    /* Now that the hash property is set, we should get one property when
     * enumerating.
     */
    propID = 0;
    numProps = 0;
    do {
        propID = CertEnumCTLContextProperties(ctl, propID);
        if (propID)
            numProps++;
    } while (propID != 0);
    ok(numProps == 1, "Expected 1 properties, got %d\n", numProps);

    checkHash(signedCTLWithCTLInnerContent,
     sizeof(signedCTLWithCTLInnerContent), CALG_MD5, ctl,
     CERT_MD5_HASH_PROP_ID);

    CertFreeCTLContext(ctl);
}

static const BYTE signedCTLWithUsage[] = {
0x30,0x82,0x01,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x00,0x30,0x81,0xfd,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,
0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x30,0x06,0x09,
0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0xa0,0x23,0x30,0x21,0x30,0x0a,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x18,0x0f,0x31,0x36,0x30,
0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x02,0x06,
0x00,0x31,0x81,0xb5,0x30,0x81,0xb2,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0xa0,0x3b,0x30,0x18,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x09,0x03,0x31,0x0b,0x06,0x09,0x2b,0x06,0x01,0x04,
0x01,0x82,0x37,0x0a,0x01,0x30,0x1f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x09,0x04,0x31,0x12,0x04,0x10,0xc4,0x3e,0x7e,0xc8,0xf9,0x85,0xf3,0x27,
0x7d,0xc0,0x38,0xb2,0x7f,0xc7,0x76,0x85,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x40,0x90,0x33,0x1b,0xb4,0x88,0x35,0xe6,0xf7,0x7f,0x93,0x05,0xc9,0x1a,0x0e,
0x8f,0x21,0xc0,0xaa,0xb3,0xab,0x3e,0x4a,0xa6,0x63,0x74,0xfd,0xef,0x11,0xbd,
0x67,0x3a,0x1b,0x07,0x4b,0x88,0x59,0x31,0xd5,0x08,0xf9,0x09,0x2f,0x0b,0x85,
0x62,0x5a,0x67,0x3b,0x62,0x7e,0x81,0x31,0xea,0xa4,0x36,0x5f,0x9a,0x92,0xb6,
0x66,0xa5,0x00,0x60,0x96 };
static const BYTE signedCTLWithListID1[] = {
0x30,0x82,0x01,0x07,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x81,0xf9,0x30,0x81,0xf6,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,
0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x29,0x06,0x09,0x2b,
0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0xa0,0x1c,0x30,0x1a,0x30,0x00,0x04,
0x01,0x01,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,
0x30,0x30,0x30,0x5a,0x30,0x02,0x06,0x00,0x31,0x81,0xb5,0x30,0x81,0xb2,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0xa0,
0x3b,0x30,0x18,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x03,0x31,
0x0b,0x06,0x09,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0x30,0x1f,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x04,0x31,0x12,0x04,0x10,0xbc,
0x13,0xde,0x17,0xc7,0x7c,0x67,0x5b,0xf9,0xa9,0x7c,0xb0,0xd9,0xf2,0x84,0xd5,
0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
static const BYTE signedCTLWithListID2[] = {
0x30,0x82,0x01,0x07,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x81,0xf9,0x30,0x81,0xf6,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,
0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x29,0x06,0x09,0x2b,
0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0xa0,0x1c,0x30,0x1a,0x30,0x00,0x04,
0x01,0x02,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,
0x30,0x30,0x30,0x5a,0x30,0x02,0x06,0x00,0x31,0x81,0xb5,0x30,0x81,0xb2,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0xa0,
0x3b,0x30,0x18,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x03,0x31,
0x0b,0x06,0x09,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x01,0x30,0x1f,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x04,0x31,0x12,0x04,0x10,0x60,
0x6b,0x10,0xb6,0xb8,0x74,0xb8,0xf3,0x79,0xd5,0x42,0x15,0x4a,0x60,0x93,0x1f,
0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };

static void testAddCTLToStore(void)
{
    HCERTSTORE store;
    BOOL ret;
    DWORD numCTLs;
    PCCTL_CONTEXT ctl;

    store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0,
     CERT_STORE_CREATE_NEW_FLAG, NULL);
    /* Add two CTLs */
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithCTLInnerContent, sizeof(signedCTLWithCTLInnerContent),
     CERT_STORE_ADD_ALWAYS, NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithCTLInnerContentAndBadSig,
     sizeof(signedCTLWithCTLInnerContentAndBadSig), CERT_STORE_ADD_ALWAYS,
     NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    /* Check that two exist */
    numCTLs = 0;
    ctl = NULL;
    do {
        ctl = CertEnumCTLsInStore(store, ctl);
        if (ctl)
            numCTLs++;
    } while (ctl);
    ok(numCTLs == 2, "expected 2 CTLs, got %d\n", numCTLs);
    CertCloseStore(store, 0);

    store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0,
     CERT_STORE_CREATE_NEW_FLAG, NULL);
    /* Add the two CTLs again.  They're identical except for the signature.. */
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithCTLInnerContent, sizeof(signedCTLWithCTLInnerContent),
     CERT_STORE_ADD_NEW, NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    /* so adding the second CTL fails. */
    SetLastError(0xdeadbeef);
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithCTLInnerContentAndBadSig,
     sizeof(signedCTLWithCTLInnerContentAndBadSig), CERT_STORE_ADD_NEW,
     NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_EXISTS ||
      GetLastError() == CRYPT_E_NOT_FOUND), /* win9x */
     "expected CRYPT_E_EXISTS, got %08x\n", GetLastError());
    CertCloseStore(store, 0);

    store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0,
     CERT_STORE_CREATE_NEW_FLAG, NULL);
    /* Add two CTLs.  These two have different usages, so they're considered
     * different.
     */
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithCTLInnerContent, sizeof(signedCTLWithCTLInnerContent),
     CERT_STORE_ADD_NEW, NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithUsage, sizeof(signedCTLWithUsage), CERT_STORE_ADD_NEW,
     NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    /* Check that two exist */
    numCTLs = 0;
    ctl = NULL;
    do {
        ctl = CertEnumCTLsInStore(store, ctl);
        if (ctl)
            numCTLs++;
    } while (ctl);
    ok(numCTLs == 2, "expected 2 CTLs, got %d\n", numCTLs);
    CertCloseStore(store, 0);

    store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0,
     CERT_STORE_CREATE_NEW_FLAG, NULL);
    /* Add two CTLs.  Now they have the same (empty) usages and different list
     * IDs, so they're different.
     */
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithListID1, sizeof(signedCTLWithListID1), CERT_STORE_ADD_NEW,
     NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    ret = CertAddEncodedCTLToStore(store, X509_ASN_ENCODING,
     signedCTLWithListID2, sizeof(signedCTLWithListID2), CERT_STORE_ADD_NEW,
     NULL);
    ok(ret, "CertAddEncodedCTLToStore failed: %08x\n", GetLastError());
    /* Check that two exist */
    numCTLs = 0;
    ctl = NULL;
    do {
        ctl = CertEnumCTLsInStore(store, ctl);
        if (ctl)
            numCTLs++;
    } while (ctl);
    ok(numCTLs == 2, "expected 2 CTLs, got %d\n", numCTLs);
    CertCloseStore(store, 0);
}

START_TEST(ctl)
{
    testCreateCTL();
    testCTLProperties();
    testAddCTLToStore();
}
