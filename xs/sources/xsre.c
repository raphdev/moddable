/*
 * Copyright (c) 2016-2018  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "xsAll.h"

//#define mxTrace 1

// COMMON

typedef struct {
	txU2 code; 
	txU1 count; 
	txU1 operand;
	txU2 delta;
} txCharCase;

static txInteger fxCharCaseCanonicalize(txInteger character, txBoolean flag);
static int fxCharCaseCompare(const void *p1, const void *p2);

enum {
	cxMatchStep,
	cxAssertionStep,
	cxAssertionCompletion,
	cxAssertionNotStep,
	cxAssertionNotCompletion,
	cxCaptureForwardStep,
	cxCaptureForwardCompletion,
	cxCaptureBackwardStep,
	cxCaptureBackwardCompletion,
	cxCaptureReferenceForwardStep,
	cxCaptureReferenceBackwardStep,
	cxCharSetForwardStep,
	cxCharSetBackwardStep,
	cxDisjunctionStep,
	cxEmptyStep,
	cxLineBeginStep,
	cxLineEndStep,
	cxQuantifierStep,
	cxQuantifierGreedyLoop,
	cxQuantifierLazyLoop,
	cxQuantifierCompletion,
	cxWordBreakStep,
	cxWordContinueStep
};

typedef txInteger (*txConditionalCharCase)(txMachine*, txInteger, txInteger);

static txInteger fxFinalSigmaToLower(txMachine* the, txInteger where, txInteger character);

#define mxConditionalCharCaseToLowerCount 1
const txConditionalCharCase gxConditionalCharCaseToLower[mxConditionalCharCaseToLowerCount] ICACHE_XS6RO_ATTR = {
	fxFinalSigmaToLower,
};
#define mxConditionalCharCaseToUpperCount 0
const txConditionalCharCase* gxConditionalCharCaseToUpper = C_NULL;

#define mxCharCaseFold0Count 198
const txCharCase gxCharCaseFold0[mxCharCaseFold0Count] ICACHE_XS6RO_ATTR = {
	{0x0041,0x1A,0x40,0x0020},{0x00B5,0x01,0x40,0x0307},{0x00C0,0x17,0x40,0x0020},{0x00D8,0x07,0x40,0x0020},{0x0100,0x2F,0x60,0x0001},{0x0132,0x05,0x60,0x0001},{0x0139,0x0F,0x50,0x0001},{0x014A,0x2D,0x60,0x0001},
	{0x0178,0x01,0x80,0x0079},{0x0179,0x05,0x50,0x0001},{0x017F,0x01,0x80,0x010C},{0x0181,0x01,0x40,0x00D2},{0x0182,0x03,0x60,0x0001},{0x0186,0x01,0x40,0x00CE},{0x0187,0x01,0x50,0x0001},{0x0189,0x02,0x40,0x00CD},
	{0x018B,0x01,0x50,0x0001},{0x018E,0x01,0x40,0x004F},{0x018F,0x01,0x40,0x00CA},{0x0190,0x01,0x40,0x00CB},{0x0191,0x01,0x50,0x0001},{0x0193,0x01,0x40,0x00CD},{0x0194,0x01,0x40,0x00CF},{0x0196,0x01,0x40,0x00D3},
	{0x0197,0x01,0x40,0x00D1},{0x0198,0x01,0x60,0x0001},{0x019C,0x01,0x40,0x00D3},{0x019D,0x01,0x40,0x00D5},{0x019F,0x01,0x40,0x00D6},{0x01A0,0x05,0x60,0x0001},{0x01A6,0x01,0x40,0x00DA},{0x01A7,0x01,0x50,0x0001},
	{0x01A9,0x01,0x40,0x00DA},{0x01AC,0x01,0x60,0x0001},{0x01AE,0x01,0x40,0x00DA},{0x01AF,0x01,0x50,0x0001},{0x01B1,0x02,0x40,0x00D9},{0x01B3,0x03,0x50,0x0001},{0x01B7,0x01,0x40,0x00DB},{0x01B8,0x01,0x60,0x0001},
	{0x01BC,0x01,0x60,0x0001},{0x01C4,0x01,0x40,0x0002},{0x01C5,0x01,0x50,0x0001},{0x01C7,0x01,0x40,0x0002},{0x01C8,0x01,0x60,0x0001},{0x01CA,0x01,0x40,0x0002},{0x01CB,0x11,0x50,0x0001},{0x01DE,0x11,0x60,0x0001},
	{0x01F1,0x01,0x40,0x0002},{0x01F2,0x03,0x60,0x0001},{0x01F6,0x01,0x80,0x0061},{0x01F7,0x01,0x80,0x0038},{0x01F8,0x27,0x60,0x0001},{0x0220,0x01,0x80,0x0082},{0x0222,0x11,0x60,0x0001},{0x023A,0x01,0x40,0x2A2B},
	{0x023B,0x01,0x50,0x0001},{0x023D,0x01,0x80,0x00A3},{0x023E,0x01,0x40,0x2A28},{0x0241,0x01,0x50,0x0001},{0x0243,0x01,0x80,0x00C3},{0x0244,0x01,0x40,0x0045},{0x0245,0x01,0x40,0x0047},{0x0246,0x09,0x60,0x0001},
	{0x0345,0x01,0x40,0x0074},{0x0370,0x03,0x60,0x0001},{0x0376,0x01,0x60,0x0001},{0x037F,0x01,0x40,0x0074},{0x0386,0x01,0x40,0x0026},{0x0388,0x03,0x40,0x0025},{0x038C,0x01,0x40,0x0040},{0x038E,0x02,0x40,0x003F},
	{0x0391,0x11,0x40,0x0020},{0x03A3,0x09,0x40,0x0020},{0x03C2,0x01,0x60,0x0001},{0x03CF,0x01,0x40,0x0008},{0x03D0,0x01,0x80,0x001E},{0x03D1,0x01,0x80,0x0019},{0x03D5,0x01,0x80,0x000F},{0x03D6,0x01,0x80,0x0016},
	{0x03D8,0x17,0x60,0x0001},{0x03F0,0x01,0x80,0x0036},{0x03F1,0x01,0x80,0x0030},{0x03F4,0x01,0x80,0x003C},{0x03F5,0x01,0x80,0x0040},{0x03F7,0x01,0x50,0x0001},{0x03F9,0x01,0x80,0x0007},{0x03FA,0x01,0x60,0x0001},
	{0x03FD,0x03,0x80,0x0082},{0x0400,0x10,0x40,0x0050},{0x0410,0x20,0x40,0x0020},{0x0460,0x21,0x60,0x0001},{0x048A,0x35,0x60,0x0001},{0x04C0,0x01,0x40,0x000F},{0x04C1,0x0D,0x50,0x0001},{0x04D0,0x5F,0x60,0x0001},
	{0x0531,0x26,0x40,0x0030},{0x10A0,0x26,0x40,0x1C60},{0x10C7,0x01,0x40,0x1C60},{0x10CD,0x01,0x40,0x1C60},{0x13F8,0x06,0x80,0x0008},{0x1C80,0x01,0x80,0x184E},{0x1C81,0x01,0x80,0x184D},{0x1C82,0x01,0x80,0x1844},
	{0x1C83,0x02,0x80,0x1842},{0x1C85,0x01,0x80,0x1843},{0x1C86,0x01,0x80,0x183C},{0x1C87,0x01,0x80,0x1824},{0x1C88,0x01,0x40,0x89C3},{0x1C90,0x2B,0x80,0x0BC0},{0x1CBD,0x03,0x80,0x0BC0},{0x1E00,0x95,0x60,0x0001},
	{0x1E9B,0x01,0x80,0x003A},{0x1E9E,0x01,0x80,0x1DBF},{0x1EA0,0x5F,0x60,0x0001},{0x1F08,0x08,0x80,0x0008},{0x1F18,0x06,0x80,0x0008},{0x1F28,0x08,0x80,0x0008},{0x1F38,0x08,0x80,0x0008},{0x1F48,0x06,0x80,0x0008},
	{0x1F59,0x01,0x80,0x0008},{0x1F5B,0x01,0x80,0x0008},{0x1F5D,0x01,0x80,0x0008},{0x1F5F,0x01,0x80,0x0008},{0x1F68,0x08,0x80,0x0008},{0x1F88,0x08,0x80,0x0008},{0x1F98,0x08,0x80,0x0008},{0x1FA8,0x08,0x80,0x0008},
	{0x1FB8,0x02,0x80,0x0008},{0x1FBA,0x02,0x80,0x004A},{0x1FBC,0x01,0x80,0x0009},{0x1FBE,0x01,0x80,0x1C05},{0x1FC8,0x04,0x80,0x0056},{0x1FCC,0x01,0x80,0x0009},{0x1FD3,0x01,0x80,0x1C43},{0x1FD8,0x02,0x80,0x0008},
	{0x1FDA,0x02,0x80,0x0064},{0x1FE3,0x01,0x80,0x1C33},{0x1FE8,0x02,0x80,0x0008},{0x1FEA,0x02,0x80,0x0070},{0x1FEC,0x01,0x80,0x0007},{0x1FF8,0x02,0x80,0x0080},{0x1FFA,0x02,0x80,0x007E},{0x1FFC,0x01,0x80,0x0009},
	{0x2126,0x01,0x80,0x1D5D},{0x212A,0x01,0x80,0x20BF},{0x212B,0x01,0x80,0x2046},{0x2132,0x01,0x40,0x001C},{0x2160,0x10,0x40,0x0010},{0x2183,0x01,0x50,0x0001},{0x24B6,0x1A,0x40,0x001A},{0x2C00,0x30,0x40,0x0030},
	{0x2C60,0x01,0x60,0x0001},{0x2C62,0x01,0x80,0x29F7},{0x2C63,0x01,0x80,0x0EE6},{0x2C64,0x01,0x80,0x29E7},{0x2C67,0x05,0x50,0x0001},{0x2C6D,0x01,0x80,0x2A1C},{0x2C6E,0x01,0x80,0x29FD},{0x2C6F,0x01,0x80,0x2A1F},
	{0x2C70,0x01,0x80,0x2A1E},{0x2C72,0x01,0x60,0x0001},{0x2C75,0x01,0x50,0x0001},{0x2C7E,0x02,0x80,0x2A3F},{0x2C80,0x63,0x60,0x0001},{0x2CEB,0x03,0x50,0x0001},{0x2CF2,0x01,0x60,0x0001},{0xA640,0x2D,0x60,0x0001},
	{0xA680,0x1B,0x60,0x0001},{0xA722,0x0D,0x60,0x0001},{0xA732,0x3D,0x60,0x0001},{0xA779,0x03,0x50,0x0001},{0xA77D,0x01,0x80,0x8A04},{0xA77E,0x09,0x60,0x0001},{0xA78B,0x01,0x50,0x0001},{0xA78D,0x01,0x80,0xA528},
	{0xA790,0x03,0x60,0x0001},{0xA796,0x13,0x60,0x0001},{0xA7AA,0x01,0x80,0xA544},{0xA7AB,0x01,0x80,0xA54F},{0xA7AC,0x01,0x80,0xA54B},{0xA7AD,0x01,0x80,0xA541},{0xA7AE,0x01,0x80,0xA544},{0xA7B0,0x01,0x80,0xA512},
	{0xA7B1,0x01,0x80,0xA52A},{0xA7B2,0x01,0x80,0xA515},{0xA7B3,0x01,0x40,0x03A0},{0xA7B4,0x0F,0x60,0x0001},{0xA7C4,0x01,0x80,0x0030},{0xA7C5,0x01,0x80,0xA543},{0xA7C6,0x01,0x80,0x8A38},{0xA7C7,0x03,0x50,0x0001},
	{0xA7D0,0x01,0x60,0x0001},{0xA7D6,0x03,0x60,0x0001},{0xA7F5,0x01,0x50,0x0001},{0xAB70,0x50,0x80,0x97D0},{0xFB05,0x01,0x50,0x0001},{0xFF21,0x1A,0x40,0x0020},
};
#define mxCharCaseFold1Count 10
const txCharCase gxCharCaseFold1[mxCharCaseFold1Count] ICACHE_XS6RO_ATTR = {
	{0x0400,0x28,0x40,0x0028},{0x04B0,0x24,0x40,0x0028},{0x0570,0x0B,0x40,0x0027},{0x057C,0x0F,0x40,0x0027},{0x058C,0x07,0x40,0x0027},{0x0594,0x02,0x40,0x0027},{0x0C80,0x33,0x40,0x0040},{0x18A0,0x20,0x40,0x0020},
	{0x6E40,0x20,0x40,0x0020},{0xE900,0x22,0x40,0x0022},
};
#define mxCharCaseIgnore0Count 191
const txCharCase gxCharCaseIgnore0[mxCharCaseIgnore0Count] ICACHE_XS6RO_ATTR = {
	{0x0061,0x1A,0x80,0x0020},{0x00B5,0x01,0x40,0x02E7},{0x00E0,0x17,0x80,0x0020},{0x00F8,0x07,0x80,0x0020},{0x00FF,0x01,0x40,0x0079},{0x0101,0x2F,0x90,0x0001},{0x0133,0x05,0x90,0x0001},{0x013A,0x0F,0xA0,0x0001},
	{0x014B,0x2D,0x90,0x0001},{0x017A,0x05,0xA0,0x0001},{0x0180,0x01,0x40,0x00C3},{0x0183,0x03,0x90,0x0001},{0x0188,0x01,0xA0,0x0001},{0x018C,0x01,0xA0,0x0001},{0x0192,0x01,0xA0,0x0001},{0x0195,0x01,0x40,0x0061},
	{0x0199,0x01,0x90,0x0001},{0x019A,0x01,0x40,0x00A3},{0x019E,0x01,0x40,0x0082},{0x01A1,0x05,0x90,0x0001},{0x01A8,0x01,0xA0,0x0001},{0x01AD,0x01,0x90,0x0001},{0x01B0,0x01,0xA0,0x0001},{0x01B4,0x03,0xA0,0x0001},
	{0x01B9,0x01,0x90,0x0001},{0x01BD,0x01,0x90,0x0001},{0x01BF,0x01,0x40,0x0038},{0x01C5,0x01,0x90,0x0001},{0x01C6,0x01,0x80,0x0002},{0x01C8,0x01,0xA0,0x0001},{0x01C9,0x01,0x80,0x0002},{0x01CB,0x01,0x90,0x0001},
	{0x01CC,0x01,0x80,0x0002},{0x01CE,0x0F,0xA0,0x0001},{0x01DD,0x01,0x80,0x004F},{0x01DF,0x11,0x90,0x0001},{0x01F2,0x01,0xA0,0x0001},{0x01F3,0x01,0x80,0x0002},{0x01F5,0x01,0x90,0x0001},{0x01F9,0x27,0x90,0x0001},
	{0x0223,0x11,0x90,0x0001},{0x023C,0x01,0xA0,0x0001},{0x023F,0x02,0x40,0x2A3F},{0x0242,0x01,0xA0,0x0001},{0x0247,0x09,0x90,0x0001},{0x0250,0x01,0x40,0x2A1F},{0x0251,0x01,0x40,0x2A1C},{0x0252,0x01,0x40,0x2A1E},
	{0x0253,0x01,0x80,0x00D2},{0x0254,0x01,0x80,0x00CE},{0x0256,0x02,0x80,0x00CD},{0x0259,0x01,0x80,0x00CA},{0x025B,0x01,0x80,0x00CB},{0x025C,0x01,0x40,0xA54F},{0x0260,0x01,0x80,0x00CD},{0x0261,0x01,0x40,0xA54B},
	{0x0263,0x01,0x80,0x00CF},{0x0265,0x01,0x40,0xA528},{0x0266,0x01,0x40,0xA544},{0x0268,0x01,0x80,0x00D1},{0x0269,0x01,0x80,0x00D3},{0x026A,0x01,0x40,0xA544},{0x026B,0x01,0x40,0x29F7},{0x026C,0x01,0x40,0xA541},
	{0x026F,0x01,0x80,0x00D3},{0x0271,0x01,0x40,0x29FD},{0x0272,0x01,0x80,0x00D5},{0x0275,0x01,0x80,0x00D6},{0x027D,0x01,0x40,0x29E7},{0x0280,0x01,0x80,0x00DA},{0x0282,0x01,0x40,0xA543},{0x0283,0x01,0x80,0x00DA},
	{0x0287,0x01,0x40,0xA52A},{0x0288,0x01,0x80,0x00DA},{0x0289,0x01,0x80,0x0045},{0x028A,0x02,0x80,0x00D9},{0x028C,0x01,0x80,0x0047},{0x0292,0x01,0x80,0x00DB},{0x029D,0x01,0x40,0xA515},{0x029E,0x01,0x40,0xA512},
	{0x0345,0x01,0x40,0x0054},{0x0371,0x03,0x90,0x0001},{0x0377,0x01,0x90,0x0001},{0x037B,0x03,0x40,0x0082},{0x03AC,0x01,0x80,0x0026},{0x03AD,0x03,0x80,0x0025},{0x03B1,0x11,0x80,0x0020},{0x03C2,0x01,0x80,0x001F},
	{0x03C3,0x09,0x80,0x0020},{0x03CC,0x01,0x80,0x0040},{0x03CD,0x02,0x80,0x003F},{0x03D0,0x01,0x80,0x003E},{0x03D1,0x01,0x80,0x0039},{0x03D5,0x01,0x80,0x002F},{0x03D6,0x01,0x80,0x0036},{0x03D7,0x01,0x80,0x0008},
	{0x03D9,0x17,0x90,0x0001},{0x03F0,0x01,0x80,0x0056},{0x03F1,0x01,0x80,0x0050},{0x03F2,0x01,0x40,0x0007},{0x03F3,0x01,0x80,0x0074},{0x03F5,0x01,0x80,0x0060},{0x03F8,0x01,0xA0,0x0001},{0x03FB,0x01,0x90,0x0001},
	{0x0430,0x20,0x80,0x0020},{0x0450,0x10,0x80,0x0050},{0x0461,0x21,0x90,0x0001},{0x048B,0x35,0x90,0x0001},{0x04C2,0x0D,0xA0,0x0001},{0x04CF,0x01,0x80,0x000F},{0x04D1,0x5F,0x90,0x0001},{0x0561,0x26,0x80,0x0030},
	{0x10D0,0x2B,0x40,0x0BC0},{0x10FD,0x03,0x40,0x0BC0},{0x13F8,0x06,0x80,0x0008},{0x1C80,0x01,0x80,0x186E},{0x1C81,0x01,0x80,0x186D},{0x1C82,0x01,0x80,0x1864},{0x1C83,0x02,0x80,0x1862},{0x1C85,0x01,0x80,0x1863},
	{0x1C86,0x01,0x80,0x185C},{0x1C87,0x01,0x80,0x1825},{0x1C88,0x01,0x40,0x89C2},{0x1D79,0x01,0x40,0x8A04},{0x1D7D,0x01,0x40,0x0EE6},{0x1D8E,0x01,0x40,0x8A38},{0x1E01,0x95,0x90,0x0001},{0x1E9B,0x01,0x80,0x003B},
	{0x1EA1,0x5F,0x90,0x0001},{0x1F00,0x08,0x40,0x0008},{0x1F10,0x06,0x40,0x0008},{0x1F20,0x08,0x40,0x0008},{0x1F30,0x08,0x40,0x0008},{0x1F40,0x06,0x40,0x0008},{0x1F51,0x01,0x40,0x0008},{0x1F53,0x01,0x40,0x0008},
	{0x1F55,0x01,0x40,0x0008},{0x1F57,0x01,0x40,0x0008},{0x1F60,0x08,0x40,0x0008},{0x1F70,0x02,0x40,0x004A},{0x1F72,0x04,0x40,0x0056},{0x1F76,0x02,0x40,0x0064},{0x1F78,0x02,0x40,0x0080},{0x1F7A,0x02,0x40,0x0070},
	{0x1F7C,0x02,0x40,0x007E},{0x1F80,0x08,0x40,0x0008},{0x1F90,0x08,0x40,0x0008},{0x1FA0,0x08,0x40,0x0008},{0x1FB0,0x02,0x40,0x0008},{0x1FB3,0x01,0x40,0x0009},{0x1FBE,0x01,0x80,0x1C25},{0x1FC3,0x01,0x40,0x0009},
	{0x1FD0,0x02,0x40,0x0008},{0x1FE0,0x02,0x40,0x0008},{0x1FE5,0x01,0x40,0x0007},{0x1FF3,0x01,0x40,0x0009},{0x214E,0x01,0x80,0x001C},{0x2170,0x10,0x80,0x0010},{0x2184,0x01,0xA0,0x0001},{0x24D0,0x1A,0x80,0x001A},
	{0x2C30,0x30,0x80,0x0030},{0x2C61,0x01,0x90,0x0001},{0x2C65,0x01,0x80,0x2A2B},{0x2C66,0x01,0x80,0x2A28},{0x2C68,0x05,0xA0,0x0001},{0x2C73,0x01,0x90,0x0001},{0x2C76,0x01,0xA0,0x0001},{0x2C81,0x63,0x90,0x0001},
	{0x2CEC,0x03,0xA0,0x0001},{0x2CF3,0x01,0x90,0x0001},{0x2D00,0x26,0x80,0x1C60},{0x2D27,0x01,0x80,0x1C60},{0x2D2D,0x01,0x80,0x1C60},{0xA641,0x2D,0x90,0x0001},{0xA681,0x1B,0x90,0x0001},{0xA723,0x0D,0x90,0x0001},
	{0xA733,0x3D,0x90,0x0001},{0xA77A,0x03,0xA0,0x0001},{0xA77F,0x09,0x90,0x0001},{0xA78C,0x01,0xA0,0x0001},{0xA791,0x03,0x90,0x0001},{0xA794,0x01,0x40,0x0030},{0xA797,0x13,0x90,0x0001},{0xA7B5,0x0F,0x90,0x0001},
	{0xA7C8,0x03,0xA0,0x0001},{0xA7D1,0x01,0x90,0x0001},{0xA7D7,0x03,0x90,0x0001},{0xA7F6,0x01,0xA0,0x0001},{0xAB53,0x01,0x80,0x03A0},{0xAB70,0x50,0x80,0x97D0},{0xFF41,0x1A,0x80,0x0020},
};
#define mxCharCaseIgnore1Count 0
const txCharCase* gxCharCaseIgnore1 = C_NULL;
#define mxSpecialCharCaseToLowerCount 2
const txInteger gxSpecialCharCaseToLower[mxSpecialCharCaseToLowerCount] ICACHE_XS6RO_ATTR = {
	0x69,0x307,
};
#define mxCharCaseToLower0Count 176
const txCharCase gxCharCaseToLower0[mxCharCaseToLower0Count] ICACHE_XS6RO_ATTR = {
	{0x0041,0x1A,0x40,0x0020},{0x00C0,0x17,0x40,0x0020},{0x00D8,0x07,0x40,0x0020},{0x0100,0x2F,0x60,0x0001},{0x0130,0x01,0x02,0x0000},{0x0132,0x05,0x60,0x0001},{0x0139,0x0F,0x50,0x0001},{0x014A,0x2D,0x60,0x0001},
	{0x0178,0x01,0x80,0x0079},{0x0179,0x05,0x50,0x0001},{0x0181,0x01,0x40,0x00D2},{0x0182,0x03,0x60,0x0001},{0x0186,0x01,0x40,0x00CE},{0x0187,0x01,0x50,0x0001},{0x0189,0x02,0x40,0x00CD},{0x018B,0x01,0x50,0x0001},
	{0x018E,0x01,0x40,0x004F},{0x018F,0x01,0x40,0x00CA},{0x0190,0x01,0x40,0x00CB},{0x0191,0x01,0x50,0x0001},{0x0193,0x01,0x40,0x00CD},{0x0194,0x01,0x40,0x00CF},{0x0196,0x01,0x40,0x00D3},{0x0197,0x01,0x40,0x00D1},
	{0x0198,0x01,0x60,0x0001},{0x019C,0x01,0x40,0x00D3},{0x019D,0x01,0x40,0x00D5},{0x019F,0x01,0x40,0x00D6},{0x01A0,0x05,0x60,0x0001},{0x01A6,0x01,0x40,0x00DA},{0x01A7,0x01,0x50,0x0001},{0x01A9,0x01,0x40,0x00DA},
	{0x01AC,0x01,0x60,0x0001},{0x01AE,0x01,0x40,0x00DA},{0x01AF,0x01,0x50,0x0001},{0x01B1,0x02,0x40,0x00D9},{0x01B3,0x03,0x50,0x0001},{0x01B7,0x01,0x40,0x00DB},{0x01B8,0x01,0x60,0x0001},{0x01BC,0x01,0x60,0x0001},
	{0x01C4,0x01,0x40,0x0002},{0x01C5,0x01,0x50,0x0001},{0x01C7,0x01,0x40,0x0002},{0x01C8,0x01,0x60,0x0001},{0x01CA,0x01,0x40,0x0002},{0x01CB,0x11,0x50,0x0001},{0x01DE,0x11,0x60,0x0001},{0x01F1,0x01,0x40,0x0002},
	{0x01F2,0x03,0x60,0x0001},{0x01F6,0x01,0x80,0x0061},{0x01F7,0x01,0x80,0x0038},{0x01F8,0x27,0x60,0x0001},{0x0220,0x01,0x80,0x0082},{0x0222,0x11,0x60,0x0001},{0x023A,0x01,0x40,0x2A2B},{0x023B,0x01,0x50,0x0001},
	{0x023D,0x01,0x80,0x00A3},{0x023E,0x01,0x40,0x2A28},{0x0241,0x01,0x50,0x0001},{0x0243,0x01,0x80,0x00C3},{0x0244,0x01,0x40,0x0045},{0x0245,0x01,0x40,0x0047},{0x0246,0x09,0x60,0x0001},{0x0370,0x03,0x60,0x0001},
	{0x0376,0x01,0x60,0x0001},{0x037F,0x01,0x40,0x0074},{0x0386,0x01,0x40,0x0026},{0x0388,0x03,0x40,0x0025},{0x038C,0x01,0x40,0x0040},{0x038E,0x02,0x40,0x003F},{0x0391,0x11,0x40,0x0020},{0x03A3,0x01,0x00,0x0000},
	{0x03A4,0x08,0x40,0x0020},{0x03CF,0x01,0x40,0x0008},{0x03D8,0x17,0x60,0x0001},{0x03F4,0x01,0x80,0x003C},{0x03F7,0x01,0x50,0x0001},{0x03F9,0x01,0x80,0x0007},{0x03FA,0x01,0x60,0x0001},{0x03FD,0x03,0x80,0x0082},
	{0x0400,0x10,0x40,0x0050},{0x0410,0x20,0x40,0x0020},{0x0460,0x21,0x60,0x0001},{0x048A,0x35,0x60,0x0001},{0x04C0,0x01,0x40,0x000F},{0x04C1,0x0D,0x50,0x0001},{0x04D0,0x5F,0x60,0x0001},{0x0531,0x26,0x40,0x0030},
	{0x10A0,0x26,0x40,0x1C60},{0x10C7,0x01,0x40,0x1C60},{0x10CD,0x01,0x40,0x1C60},{0x13A0,0x50,0x40,0x97D0},{0x13F0,0x06,0x40,0x0008},{0x1C90,0x2B,0x80,0x0BC0},{0x1CBD,0x03,0x80,0x0BC0},{0x1E00,0x95,0x60,0x0001},
	{0x1E9E,0x01,0x80,0x1DBF},{0x1EA0,0x5F,0x60,0x0001},{0x1F08,0x08,0x80,0x0008},{0x1F18,0x06,0x80,0x0008},{0x1F28,0x08,0x80,0x0008},{0x1F38,0x08,0x80,0x0008},{0x1F48,0x06,0x80,0x0008},{0x1F59,0x01,0x80,0x0008},
	{0x1F5B,0x01,0x80,0x0008},{0x1F5D,0x01,0x80,0x0008},{0x1F5F,0x01,0x80,0x0008},{0x1F68,0x08,0x80,0x0008},{0x1F88,0x08,0x80,0x0008},{0x1F98,0x08,0x80,0x0008},{0x1FA8,0x08,0x80,0x0008},{0x1FB8,0x02,0x80,0x0008},
	{0x1FBA,0x02,0x80,0x004A},{0x1FBC,0x01,0x80,0x0009},{0x1FC8,0x04,0x80,0x0056},{0x1FCC,0x01,0x80,0x0009},{0x1FD8,0x02,0x80,0x0008},{0x1FDA,0x02,0x80,0x0064},{0x1FE8,0x02,0x80,0x0008},{0x1FEA,0x02,0x80,0x0070},
	{0x1FEC,0x01,0x80,0x0007},{0x1FF8,0x02,0x80,0x0080},{0x1FFA,0x02,0x80,0x007E},{0x1FFC,0x01,0x80,0x0009},{0x2126,0x01,0x80,0x1D5D},{0x212A,0x01,0x80,0x20BF},{0x212B,0x01,0x80,0x2046},{0x2132,0x01,0x40,0x001C},
	{0x2160,0x10,0x40,0x0010},{0x2183,0x01,0x50,0x0001},{0x24B6,0x1A,0x40,0x001A},{0x2C00,0x30,0x40,0x0030},{0x2C60,0x01,0x60,0x0001},{0x2C62,0x01,0x80,0x29F7},{0x2C63,0x01,0x80,0x0EE6},{0x2C64,0x01,0x80,0x29E7},
	{0x2C67,0x05,0x50,0x0001},{0x2C6D,0x01,0x80,0x2A1C},{0x2C6E,0x01,0x80,0x29FD},{0x2C6F,0x01,0x80,0x2A1F},{0x2C70,0x01,0x80,0x2A1E},{0x2C72,0x01,0x60,0x0001},{0x2C75,0x01,0x50,0x0001},{0x2C7E,0x02,0x80,0x2A3F},
	{0x2C80,0x63,0x60,0x0001},{0x2CEB,0x03,0x50,0x0001},{0x2CF2,0x01,0x60,0x0001},{0xA640,0x2D,0x60,0x0001},{0xA680,0x1B,0x60,0x0001},{0xA722,0x0D,0x60,0x0001},{0xA732,0x3D,0x60,0x0001},{0xA779,0x03,0x50,0x0001},
	{0xA77D,0x01,0x80,0x8A04},{0xA77E,0x09,0x60,0x0001},{0xA78B,0x01,0x50,0x0001},{0xA78D,0x01,0x80,0xA528},{0xA790,0x03,0x60,0x0001},{0xA796,0x13,0x60,0x0001},{0xA7AA,0x01,0x80,0xA544},{0xA7AB,0x01,0x80,0xA54F},
	{0xA7AC,0x01,0x80,0xA54B},{0xA7AD,0x01,0x80,0xA541},{0xA7AE,0x01,0x80,0xA544},{0xA7B0,0x01,0x80,0xA512},{0xA7B1,0x01,0x80,0xA52A},{0xA7B2,0x01,0x80,0xA515},{0xA7B3,0x01,0x40,0x03A0},{0xA7B4,0x0F,0x60,0x0001},
	{0xA7C4,0x01,0x80,0x0030},{0xA7C5,0x01,0x80,0xA543},{0xA7C6,0x01,0x80,0x8A38},{0xA7C7,0x03,0x50,0x0001},{0xA7D0,0x01,0x60,0x0001},{0xA7D6,0x03,0x60,0x0001},{0xA7F5,0x01,0x50,0x0001},{0xFF21,0x1A,0x40,0x0020},
};
#define mxCharCaseToLower1Count 10
const txCharCase gxCharCaseToLower1[mxCharCaseToLower1Count] ICACHE_XS6RO_ATTR = {
	{0x0400,0x28,0x40,0x0028},{0x04B0,0x24,0x40,0x0028},{0x0570,0x0B,0x40,0x0027},{0x057C,0x0F,0x40,0x0027},{0x058C,0x07,0x40,0x0027},{0x0594,0x02,0x40,0x0027},{0x0C80,0x33,0x40,0x0040},{0x18A0,0x20,0x40,0x0020},
	{0x6E40,0x20,0x40,0x0020},{0xE900,0x22,0x40,0x0022},
};
#define mxSpecialCharCaseToUpperCount 220
const txInteger gxSpecialCharCaseToUpper[mxSpecialCharCaseToUpperCount] ICACHE_XS6RO_ATTR = {
	0x53,0x53,0x46,0x46,0x46,0x49,0x46,0x4c,0x46,0x46,0x49,0x46,0x46,0x4c,0x53,0x54,0x53,0x54,0x535,0x552,0x544,0x546,0x544,0x535,0x544,0x53b,0x54e,0x546,0x544,0x53d,0x2bc,0x4e,
	0x399,0x308,0x301,0x3a5,0x308,0x301,0x4a,0x30c,0x48,0x331,0x54,0x308,0x57,0x30a,0x59,0x30a,0x41,0x2be,0x3a5,0x313,0x3a5,0x313,0x300,0x3a5,0x313,0x301,0x3a5,0x313,0x342,0x391,0x342,0x397,
	0x342,0x399,0x308,0x300,0x399,0x308,0x301,0x399,0x342,0x399,0x308,0x342,0x3a5,0x308,0x300,0x3a5,0x308,0x301,0x3a1,0x313,0x3a5,0x342,0x3a5,0x308,0x342,0x3a9,0x342,0x1f08,0x399,0x1f09,0x399,0x1f0a,
	0x399,0x1f0b,0x399,0x1f0c,0x399,0x1f0d,0x399,0x1f0e,0x399,0x1f0f,0x399,0x1f08,0x399,0x1f09,0x399,0x1f0a,0x399,0x1f0b,0x399,0x1f0c,0x399,0x1f0d,0x399,0x1f0e,0x399,0x1f0f,0x399,0x1f28,0x399,0x1f29,0x399,0x1f2a,
	0x399,0x1f2b,0x399,0x1f2c,0x399,0x1f2d,0x399,0x1f2e,0x399,0x1f2f,0x399,0x1f28,0x399,0x1f29,0x399,0x1f2a,0x399,0x1f2b,0x399,0x1f2c,0x399,0x1f2d,0x399,0x1f2e,0x399,0x1f2f,0x399,0x1f68,0x399,0x1f69,0x399,0x1f6a,
	0x399,0x1f6b,0x399,0x1f6c,0x399,0x1f6d,0x399,0x1f6e,0x399,0x1f6f,0x399,0x1f68,0x399,0x1f69,0x399,0x1f6a,0x399,0x1f6b,0x399,0x1f6c,0x399,0x1f6d,0x399,0x1f6e,0x399,0x1f6f,0x399,0x391,0x399,0x391,0x399,0x397,
	0x399,0x397,0x399,0x3a9,0x399,0x3a9,0x399,0x1fba,0x399,0x386,0x399,0x1fca,0x399,0x389,0x399,0x1ffa,0x399,0x38f,0x399,0x391,0x342,0x399,0x397,0x342,0x399,0x3a9,0x342,0x399,
};
#define mxCharCaseToUpper0Count 289
const txCharCase gxCharCaseToUpper0[mxCharCaseToUpper0Count] ICACHE_XS6RO_ATTR = {
	{0x0061,0x1A,0x80,0x0020},{0x00B5,0x01,0x40,0x02E7},{0x00DF,0x01,0x02,0x0000},{0x00E0,0x17,0x80,0x0020},{0x00F8,0x07,0x80,0x0020},{0x00FF,0x01,0x40,0x0079},{0x0101,0x2F,0x90,0x0001},{0x0131,0x01,0x80,0x00E8},
	{0x0133,0x05,0x90,0x0001},{0x013A,0x0F,0xA0,0x0001},{0x0149,0x01,0x02,0x001E},{0x014B,0x2D,0x90,0x0001},{0x017A,0x05,0xA0,0x0001},{0x017F,0x01,0x80,0x012C},{0x0180,0x01,0x40,0x00C3},{0x0183,0x03,0x90,0x0001},
	{0x0188,0x01,0xA0,0x0001},{0x018C,0x01,0xA0,0x0001},{0x0192,0x01,0xA0,0x0001},{0x0195,0x01,0x40,0x0061},{0x0199,0x01,0x90,0x0001},{0x019A,0x01,0x40,0x00A3},{0x019E,0x01,0x40,0x0082},{0x01A1,0x05,0x90,0x0001},
	{0x01A8,0x01,0xA0,0x0001},{0x01AD,0x01,0x90,0x0001},{0x01B0,0x01,0xA0,0x0001},{0x01B4,0x03,0xA0,0x0001},{0x01B9,0x01,0x90,0x0001},{0x01BD,0x01,0x90,0x0001},{0x01BF,0x01,0x40,0x0038},{0x01C5,0x01,0x90,0x0001},
	{0x01C6,0x01,0x80,0x0002},{0x01C8,0x01,0xA0,0x0001},{0x01C9,0x01,0x80,0x0002},{0x01CB,0x01,0x90,0x0001},{0x01CC,0x01,0x80,0x0002},{0x01CE,0x0F,0xA0,0x0001},{0x01DD,0x01,0x80,0x004F},{0x01DF,0x11,0x90,0x0001},
	{0x01F0,0x01,0x02,0x0026},{0x01F2,0x01,0xA0,0x0001},{0x01F3,0x01,0x80,0x0002},{0x01F5,0x01,0x90,0x0001},{0x01F9,0x27,0x90,0x0001},{0x0223,0x11,0x90,0x0001},{0x023C,0x01,0xA0,0x0001},{0x023F,0x02,0x40,0x2A3F},
	{0x0242,0x01,0xA0,0x0001},{0x0247,0x09,0x90,0x0001},{0x0250,0x01,0x40,0x2A1F},{0x0251,0x01,0x40,0x2A1C},{0x0252,0x01,0x40,0x2A1E},{0x0253,0x01,0x80,0x00D2},{0x0254,0x01,0x80,0x00CE},{0x0256,0x02,0x80,0x00CD},
	{0x0259,0x01,0x80,0x00CA},{0x025B,0x01,0x80,0x00CB},{0x025C,0x01,0x40,0xA54F},{0x0260,0x01,0x80,0x00CD},{0x0261,0x01,0x40,0xA54B},{0x0263,0x01,0x80,0x00CF},{0x0265,0x01,0x40,0xA528},{0x0266,0x01,0x40,0xA544},
	{0x0268,0x01,0x80,0x00D1},{0x0269,0x01,0x80,0x00D3},{0x026A,0x01,0x40,0xA544},{0x026B,0x01,0x40,0x29F7},{0x026C,0x01,0x40,0xA541},{0x026F,0x01,0x80,0x00D3},{0x0271,0x01,0x40,0x29FD},{0x0272,0x01,0x80,0x00D5},
	{0x0275,0x01,0x80,0x00D6},{0x027D,0x01,0x40,0x29E7},{0x0280,0x01,0x80,0x00DA},{0x0282,0x01,0x40,0xA543},{0x0283,0x01,0x80,0x00DA},{0x0287,0x01,0x40,0xA52A},{0x0288,0x01,0x80,0x00DA},{0x0289,0x01,0x80,0x0045},
	{0x028A,0x02,0x80,0x00D9},{0x028C,0x01,0x80,0x0047},{0x0292,0x01,0x80,0x00DB},{0x029D,0x01,0x40,0xA515},{0x029E,0x01,0x40,0xA512},{0x0345,0x01,0x40,0x0054},{0x0371,0x03,0x90,0x0001},{0x0377,0x01,0x90,0x0001},
	{0x037B,0x03,0x40,0x0082},{0x0390,0x01,0x03,0x0020},{0x03AC,0x01,0x80,0x0026},{0x03AD,0x03,0x80,0x0025},{0x03B0,0x01,0x03,0x0023},{0x03B1,0x11,0x80,0x0020},{0x03C2,0x01,0x80,0x001F},{0x03C3,0x09,0x80,0x0020},
	{0x03CC,0x01,0x80,0x0040},{0x03CD,0x02,0x80,0x003F},{0x03D0,0x01,0x80,0x003E},{0x03D1,0x01,0x80,0x0039},{0x03D5,0x01,0x80,0x002F},{0x03D6,0x01,0x80,0x0036},{0x03D7,0x01,0x80,0x0008},{0x03D9,0x17,0x90,0x0001},
	{0x03F0,0x01,0x80,0x0056},{0x03F1,0x01,0x80,0x0050},{0x03F2,0x01,0x40,0x0007},{0x03F3,0x01,0x80,0x0074},{0x03F5,0x01,0x80,0x0060},{0x03F8,0x01,0xA0,0x0001},{0x03FB,0x01,0x90,0x0001},{0x0430,0x20,0x80,0x0020},
	{0x0450,0x10,0x80,0x0050},{0x0461,0x21,0x90,0x0001},{0x048B,0x35,0x90,0x0001},{0x04C2,0x0D,0xA0,0x0001},{0x04CF,0x01,0x80,0x000F},{0x04D1,0x5F,0x90,0x0001},{0x0561,0x26,0x80,0x0030},{0x0587,0x01,0x02,0x0012},
	{0x10D0,0x2B,0x40,0x0BC0},{0x10FD,0x03,0x40,0x0BC0},{0x13F8,0x06,0x80,0x0008},{0x1C80,0x01,0x80,0x186E},{0x1C81,0x01,0x80,0x186D},{0x1C82,0x01,0x80,0x1864},{0x1C83,0x02,0x80,0x1862},{0x1C85,0x01,0x80,0x1863},
	{0x1C86,0x01,0x80,0x185C},{0x1C87,0x01,0x80,0x1825},{0x1C88,0x01,0x40,0x89C2},{0x1D79,0x01,0x40,0x8A04},{0x1D7D,0x01,0x40,0x0EE6},{0x1D8E,0x01,0x40,0x8A38},{0x1E01,0x95,0x90,0x0001},{0x1E96,0x01,0x02,0x0028},
	{0x1E97,0x01,0x02,0x002A},{0x1E98,0x01,0x02,0x002C},{0x1E99,0x01,0x02,0x002E},{0x1E9A,0x01,0x02,0x0030},{0x1E9B,0x01,0x80,0x003B},{0x1EA1,0x5F,0x90,0x0001},{0x1F00,0x08,0x40,0x0008},{0x1F10,0x06,0x40,0x0008},
	{0x1F20,0x08,0x40,0x0008},{0x1F30,0x08,0x40,0x0008},{0x1F40,0x06,0x40,0x0008},{0x1F50,0x01,0x02,0x0032},{0x1F51,0x01,0x40,0x0008},{0x1F52,0x01,0x03,0x0034},{0x1F53,0x01,0x40,0x0008},{0x1F54,0x01,0x03,0x0037},
	{0x1F55,0x01,0x40,0x0008},{0x1F56,0x01,0x03,0x003A},{0x1F57,0x01,0x40,0x0008},{0x1F60,0x08,0x40,0x0008},{0x1F70,0x02,0x40,0x004A},{0x1F72,0x04,0x40,0x0056},{0x1F76,0x02,0x40,0x0064},{0x1F78,0x02,0x40,0x0080},
	{0x1F7A,0x02,0x40,0x0070},{0x1F7C,0x02,0x40,0x007E},{0x1F80,0x01,0x02,0x005B},{0x1F81,0x01,0x02,0x005D},{0x1F82,0x01,0x02,0x005F},{0x1F83,0x01,0x02,0x0061},{0x1F84,0x01,0x02,0x0063},{0x1F85,0x01,0x02,0x0065},
	{0x1F86,0x01,0x02,0x0067},{0x1F87,0x01,0x02,0x0069},{0x1F88,0x01,0x02,0x006B},{0x1F89,0x01,0x02,0x006D},{0x1F8A,0x01,0x02,0x006F},{0x1F8B,0x01,0x02,0x0071},{0x1F8C,0x01,0x02,0x0073},{0x1F8D,0x01,0x02,0x0075},
	{0x1F8E,0x01,0x02,0x0077},{0x1F8F,0x01,0x02,0x0079},{0x1F90,0x01,0x02,0x007B},{0x1F91,0x01,0x02,0x007D},{0x1F92,0x01,0x02,0x007F},{0x1F93,0x01,0x02,0x0081},{0x1F94,0x01,0x02,0x0083},{0x1F95,0x01,0x02,0x0085},
	{0x1F96,0x01,0x02,0x0087},{0x1F97,0x01,0x02,0x0089},{0x1F98,0x01,0x02,0x008B},{0x1F99,0x01,0x02,0x008D},{0x1F9A,0x01,0x02,0x008F},{0x1F9B,0x01,0x02,0x0091},{0x1F9C,0x01,0x02,0x0093},{0x1F9D,0x01,0x02,0x0095},
	{0x1F9E,0x01,0x02,0x0097},{0x1F9F,0x01,0x02,0x0099},{0x1FA0,0x01,0x02,0x009B},{0x1FA1,0x01,0x02,0x009D},{0x1FA2,0x01,0x02,0x009F},{0x1FA3,0x01,0x02,0x00A1},{0x1FA4,0x01,0x02,0x00A3},{0x1FA5,0x01,0x02,0x00A5},
	{0x1FA6,0x01,0x02,0x00A7},{0x1FA7,0x01,0x02,0x00A9},{0x1FA8,0x01,0x02,0x00AB},{0x1FA9,0x01,0x02,0x00AD},{0x1FAA,0x01,0x02,0x00AF},{0x1FAB,0x01,0x02,0x00B1},{0x1FAC,0x01,0x02,0x00B3},{0x1FAD,0x01,0x02,0x00B5},
	{0x1FAE,0x01,0x02,0x00B7},{0x1FAF,0x01,0x02,0x00B9},{0x1FB0,0x02,0x40,0x0008},{0x1FB2,0x01,0x02,0x00C7},{0x1FB3,0x01,0x02,0x00BB},{0x1FB4,0x01,0x02,0x00C9},{0x1FB6,0x01,0x02,0x003D},{0x1FB7,0x01,0x03,0x00D3},
	{0x1FBC,0x01,0x02,0x00BD},{0x1FBE,0x01,0x80,0x1C25},{0x1FC2,0x01,0x02,0x00CB},{0x1FC3,0x01,0x02,0x00BF},{0x1FC4,0x01,0x02,0x00CD},{0x1FC6,0x01,0x02,0x003F},{0x1FC7,0x01,0x03,0x00D6},{0x1FCC,0x01,0x02,0x00C1},
	{0x1FD0,0x02,0x40,0x0008},{0x1FD2,0x01,0x03,0x0041},{0x1FD3,0x01,0x03,0x0044},{0x1FD6,0x01,0x02,0x0047},{0x1FD7,0x01,0x03,0x0049},{0x1FE0,0x02,0x40,0x0008},{0x1FE2,0x01,0x03,0x004C},{0x1FE3,0x01,0x03,0x004F},
	{0x1FE4,0x01,0x02,0x0052},{0x1FE5,0x01,0x40,0x0007},{0x1FE6,0x01,0x02,0x0054},{0x1FE7,0x01,0x03,0x0056},{0x1FF2,0x01,0x02,0x00CF},{0x1FF3,0x01,0x02,0x00C3},{0x1FF4,0x01,0x02,0x00D1},{0x1FF6,0x01,0x02,0x0059},
	{0x1FF7,0x01,0x03,0x00D9},{0x1FFC,0x01,0x02,0x00C5},{0x214E,0x01,0x80,0x001C},{0x2170,0x10,0x80,0x0010},{0x2184,0x01,0xA0,0x0001},{0x24D0,0x1A,0x80,0x001A},{0x2C30,0x30,0x80,0x0030},{0x2C61,0x01,0x90,0x0001},
	{0x2C65,0x01,0x80,0x2A2B},{0x2C66,0x01,0x80,0x2A28},{0x2C68,0x05,0xA0,0x0001},{0x2C73,0x01,0x90,0x0001},{0x2C76,0x01,0xA0,0x0001},{0x2C81,0x63,0x90,0x0001},{0x2CEC,0x03,0xA0,0x0001},{0x2CF3,0x01,0x90,0x0001},
	{0x2D00,0x26,0x80,0x1C60},{0x2D27,0x01,0x80,0x1C60},{0x2D2D,0x01,0x80,0x1C60},{0xA641,0x2D,0x90,0x0001},{0xA681,0x1B,0x90,0x0001},{0xA723,0x0D,0x90,0x0001},{0xA733,0x3D,0x90,0x0001},{0xA77A,0x03,0xA0,0x0001},
	{0xA77F,0x09,0x90,0x0001},{0xA78C,0x01,0xA0,0x0001},{0xA791,0x03,0x90,0x0001},{0xA794,0x01,0x40,0x0030},{0xA797,0x13,0x90,0x0001},{0xA7B5,0x0F,0x90,0x0001},{0xA7C8,0x03,0xA0,0x0001},{0xA7D1,0x01,0x90,0x0001},
	{0xA7D7,0x03,0x90,0x0001},{0xA7F6,0x01,0xA0,0x0001},{0xAB53,0x01,0x80,0x03A0},{0xAB70,0x50,0x80,0x97D0},{0xFB00,0x01,0x02,0x0002},{0xFB01,0x01,0x02,0x0004},{0xFB02,0x01,0x02,0x0006},{0xFB03,0x01,0x03,0x0008},
	{0xFB04,0x01,0x03,0x000B},{0xFB05,0x01,0x02,0x000E},{0xFB06,0x01,0x02,0x0010},{0xFB13,0x01,0x02,0x0014},{0xFB14,0x01,0x02,0x0016},{0xFB15,0x01,0x02,0x0018},{0xFB16,0x01,0x02,0x001A},{0xFB17,0x01,0x02,0x001C},
	{0xFF41,0x1A,0x80,0x0020},
};
#define mxCharCaseToUpper1Count 10
const txCharCase gxCharCaseToUpper1[mxCharCaseToUpper1Count] ICACHE_XS6RO_ATTR = {
	{0x0428,0x28,0x80,0x0028},{0x04D8,0x24,0x80,0x0028},{0x0597,0x0B,0x80,0x0027},{0x05A3,0x0F,0x80,0x0027},{0x05B3,0x07,0x80,0x0027},{0x05BB,0x02,0x80,0x0027},{0x0CC0,0x33,0x80,0x0040},{0x18C0,0x20,0x80,0x0020},
	{0x6E60,0x20,0x80,0x0020},{0xE922,0x22,0x80,0x0022},
};

// COMPILE

typedef struct sxPatternParser txPatternParser;

typedef void (*txTermMeasure)(txPatternParser*, void*, txInteger);
typedef void (*txTermCode)(txPatternParser*, void*, txInteger, txInteger);

#define txTermPart\
	void* next;\
	union {\
		txTermMeasure measure;\
		txTermCode code;\
	} dispatch;\
	txInteger step

typedef struct {
	txTermPart;
} txTerm;

typedef struct {
	txTermPart;
	txTerm* term;
	txBoolean not;
	txInteger direction;
	txInteger assertionIndex;
	txInteger completion;
} txAssertion;

typedef struct sxCapture txCapture;
struct sxCapture {
	txTermPart;
	txTerm* term;
	txInteger captureIndex;
	txInteger completion;
	txCapture* nextNamedCapture;
	char name[1];
};

typedef struct {
	txTermPart;
	txInteger captureIndex;
	char name[1];
} txCaptureReference;

typedef struct {
	txTermPart;
	txInteger stringCount;
	txString* strings;
	txInteger characters[1];
} txCharSet;

enum {
	mxCharSetUnionOp = 0,
	mxCharSetSubtractOp = 1,
	mxCharSetIntersectionOp = 3,
	mxCharSetRangeOp = 4,
};

#define mxCharSetStrings(CHARSET) \
	((txString)CHARSET + sizeof(txCharSet) + (CHARSET->characters[0] * sizeof(txInteger)))

typedef struct {
	txTermPart;
	txTerm* left;
	txTerm* right;
} txDisjunction;

typedef struct {
	txTermPart;
	txTerm* term;
	txInteger min;
	txInteger max;
	txBoolean greedy;
	txInteger captureIndex;
	txInteger captureCount;
	txInteger quantifierIndex;
	txInteger loop;
	txInteger completion;
} txQuantifier;

typedef struct {
	txTermPart;
	txTerm* left;
	txTerm* right;
} txSequence;

enum {
	XS_REGEXP_NAME = 1 << 9,
};

struct sxPatternParser {
	txMachine* the;
	txTerm* first;
	
	txUnsigned flags;
	txString pattern;
	txInteger offset;
	txInteger character;
	txInteger surrogate;
		
	txInteger assertionIndex;
	txInteger captureIndex;
	txInteger quantifierIndex;
	txCapture* firstNamedCapture;
	
	txInteger size;

	txInteger** code;
	txByte* buffer;
	
	char* stackLimit;
	c_jmp_buf jmp_buf;
	char error[256];
};

static void* fxCharSetAny(txPatternParser* parser);
static void* fxCharSetCombine(txPatternParser* parser, txCharSet* set1, txCharSet* set2, txInteger op);
static void* fxCharSetDigits(txPatternParser* parser);
static void* fxCharSetEmpty(txPatternParser* parser);
static void* fxCharSetExpression(txPatternParser* parser);
static void* fxCharSetNot(txPatternParser* parser, txCharSet* set);
static void* fxCharSetOperand(txPatternParser* parser, txInteger* kind);
static void* fxCharSetParseEscape(txPatternParser* parser, txBoolean punctuator, txInteger* kind);
static void* fxCharSetParseItem(txPatternParser* parser);
static void* fxCharSetParseList(txPatternParser* parser);
static void* fxCharSetRange(txPatternParser* parser, txCharSet* set1, txCharSet* set2);
static void* fxCharSetSingle(txPatternParser* parser, txInteger character);
static void* fxCharSetSpaces(txPatternParser* parser);
static void* fxCharSetStrings(txPatternParser* parser);
static void* fxCharSetStringsDisjunction(txPatternParser* parser, txCharSet* set);
static void* fxCharSetStringsSequence(txPatternParser* parser, txString string);
static void* fxCharSetUnicodeProperty(txPatternParser* parser);
static void* fxCharSetWords(txPatternParser* parser);
static void* fxDisjunctionParse(txPatternParser* parser, txInteger character);
static void* fxQuantifierParse(txPatternParser* parser, void* term, txInteger captureIndex);
static txBoolean fxQuantifierParseBrace(txPatternParser* parser, txInteger* min, txInteger* max);
static txBoolean fxQuantifierParseDigits(txPatternParser* parser, txInteger* result);
static void* fxSequenceParse(txPatternParser* parser, txInteger character);
static void* fxPatternParserCreateTerm(txPatternParser* parser, size_t size, txTermMeasure measure);

static void fxAssertionMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxCaptureMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxCaptureReferenceMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxCharSetMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxDisjunctionMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxEmptyMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxLineBeginMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxLineEndMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxQuantifierMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxSequenceMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxWordBreakMeasure(txPatternParser* parser, void* it, txInteger direction);
static void fxWordContinueMeasure(txPatternParser* parser, void* it, txInteger direction);

static void fxAssertionCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxCaptureCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxCaptureReferenceCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxCharSetCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxDisjunctionCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxEmptyCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxLineBeginCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxLineEndCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxQuantifierCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxSequenceCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxWordBreakCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);
static void fxWordContinueCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel);

static void fxPatternParserCheckStack(txPatternParser* parser);
static void* fxPatternParserCreateChunk(txPatternParser* parser, txSize size);
static void fxPatternParserInitialize(txPatternParser* parser);
static txBoolean fxPatternParserDecimal(txPatternParser* parser, txU4* value);
static void fxPatternParserError(txPatternParser* parser, txString format, ...);
static void fxPatternParserEscape(txPatternParser* parser, txBoolean punctuator);
static void fxPatternParserName(txPatternParser* parser, txInteger* length);
static void fxPatternParserNameEscape(txPatternParser* parser);
static void fxPatternParserNamedCapture(txPatternParser* parser, txCapture* capture);
static void fxPatternParserNext(txPatternParser* parser);
static void fxPatternParserTerminate(txPatternParser* parser);

#define mxCodeSize sizeof(txInteger)
#define mxStepSize sizeof(txInteger)
#define mxIndexSize sizeof(txInteger)
#define mxTermStepSize mxCodeSize + mxStepSize
#define mxAssertionNotStepSize mxTermStepSize + mxIndexSize + mxStepSize
#define mxAssertionNotCompletionSize mxCodeSize + mxIndexSize
#define mxAssertionStepSize mxTermStepSize + mxIndexSize
#define mxAssertionCompletionSize mxTermStepSize + mxIndexSize
#define mxCaptureStepSize mxTermStepSize + mxIndexSize
#define mxCaptureCompletionSize mxTermStepSize + mxIndexSize
#define mxCaptureReferenceStepSize mxTermStepSize + mxIndexSize
#define mxDisjunctionStepSize mxTermStepSize + mxStepSize
#define mxQuantifierStepSize mxTermStepSize + mxIndexSize + (2 * sizeof(txInteger))
#define mxQuantifierLoopSize mxTermStepSize + mxIndexSize + mxStepSize + mxIndexSize + mxIndexSize
#define mxQuantifierCompletionSize mxTermStepSize + mxIndexSize + mxStepSize

enum {
	mxDuplicateCapture,
	mxInvalidCharacter,
	mxInvalidEscape,
	mxInvalidFlags,
	mxInvalidGroup,
	mxInvalidName,
	mxInvalidPattern,
	mxInvalidQuantifier,
	mxInvalidRange,
	mxInvalidReferenceName,
	mxInvalidReferenceNumber,
	mxInvalidSequence,
	mxInvalidUTF8,
	mxNameOverflow,
	mxNotEnoughMemory,
	mxStackOverflow,
	mxUnicodePropertyEscapeNotBuiltIn,
	mxErrorCount
};
static const txString gxErrors[mxErrorCount] ICACHE_XS6RO_ATTR = {
	"duplicate capture",
	"invalid character",
	"invalid escape",
	"invalid flags",
	"invalid group",
	"invalid name",
	"invalid pattern",
	"invalid quantifier",
	"invalid range",
	"invalid reference name \\k<%s>",
	"invalid reference number \\%d",
	"invalid sequence",
	"invalid UTF-8",
	"name overflow",
	"not enough memory",
	"stack oveflow",
	"unicode property escape not built-in",
};

// MATCH

typedef struct sxStateData txStateData;

typedef struct {
	txInteger offset;
	txStateData* firstState;
} txAssertionData;

typedef struct {
	txInteger from;
	txInteger to;
} txCaptureData;

typedef struct {
	txInteger min;
	txInteger max;
	txInteger offset;
} txQuantifierData;

struct sxStateData {
	txMachine* the;
	txStateData* nextState;
	txInteger step;
	txInteger offset;
	txCaptureData captures[1];
};

static txInteger fxFindCharacter(txString input, txInteger offset, txInteger direction, txInteger flags);
static txInteger fxGetCharacter(txString input, txInteger offset, txInteger flags);
static txBoolean fxMatchCharacter(txInteger* characters, txInteger character);
static txStateData* fxPopStates(txMachine* the, txStateData* fromState, txStateData* toState);
static txStateData* fxPushState(txMachine* the, txStateData* firstState, txInteger step, txInteger offset, txCaptureData* captures, txInteger captureCount);

#ifdef mxTrace
	static txString gxStepNames[cxWordContinueStep + 1] = {
		"MatchStep",
		"AssertionStep",
		"AssertionCompletion",
		"AssertionNotStep",
		"AssertionNotCompletion",
		"CaptureForwardStep",
		"CaptureForwardCompletion",
		"CaptureBackwardStep",
		"CaptureBackwardCompletion",
		"CaptureReferenceForwardStep",
		"CaptureReferenceBackwardStep",
		"CharSetForwardStep",
		"CharSetBackwardStep",
		"DisjunctionStep",
		"EmptyStep",
		"LineBeginStep",
		"LineEndStep",
		"QuantifierStep",
		"QuantifierGreedyLoop",
		"QuantifierLazyLoop",
		"QuantifierCompletion",
		"WordBreakStep",
		"WordContinueStep"
	};
#endif

static const txInteger gxLineCharacters[7] ICACHE_XS6RO_ATTR = { 6, 0x000A, 0x000A + 1, 0x000D, 0x000D + 1, 0x2028, 0x2029 + 1 };
static const txInteger gxWordCharacters[9] ICACHE_XS6RO_ATTR = { 8, '0', '9' + 1, 'A', 'Z' + 1, '_', '_' + 1, 'a', 'z' + 1 };

// COMMON

txInteger fxCharCaseCanonicalize(txInteger character, txBoolean flag)
{
	txCharCase charCase;
	txCharCase* it;
	if (character < 0x80) {
		if (flag)
			it = (txCharCase*)gxCharCaseFold0;
		else
			it = (txCharCase*)gxCharCaseIgnore0;
		if (character < it->code)
			it = C_NULL;
		else if ((it->code + it->count) <= character)
			it = C_NULL;
	}
	else if (character < 0x10000) {
		charCase.code = (txU2)character;
		charCase.count = 1;
		if (flag)
			it = (txCharCase*)bsearch(&charCase, gxCharCaseFold0, mxCharCaseFold0Count, sizeof(txCharCase), fxCharCaseCompare);
		else
			it = (txCharCase*)bsearch(&charCase, gxCharCaseIgnore0, mxCharCaseIgnore0Count, sizeof(txCharCase), fxCharCaseCompare);
		
	}
	else {
		charCase.code = (txU2)(character & 0xFFFF);
		charCase.count = 1;
		if (flag)
			it = (txCharCase*)bsearch(&charCase, gxCharCaseFold1, mxCharCaseFold1Count, sizeof(txCharCase), fxCharCaseCompare);
		else
			it = C_NULL;
	}
	if (it) {
		txU1 operand = it->operand;
		if ((operand & 0x10) && ((character & 1) == 0))
			return character;
		if ((operand & 0x20) && (character & 1))
			return character;
		if (operand & 0x40)
			character += it->delta;
		else if (operand & 0x80)
			character -= it->delta;
	}
	return character;
}

int fxCharCaseCompare(const void *p1, const void *p2)
{
	txCharCase* c1 = (txCharCase*)p1;
	txCharCase* c2 = (txCharCase*)p2;
	if (c1->code + c1->count <= c2->code)
		return -1;
	if (c1->code >= c2->code + c2->count)
		return 1;
	return 0;
}

// COMPILE

void* fxCharSetAny(txPatternParser* parser)
{
	txCharSet* result;
	if (parser->flags & XS_REGEXP_S) {
		result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (2 * sizeof(txInteger)), fxCharSetMeasure);
		result->stringCount = 0;
		result->strings = C_NULL;
		result->characters[0] = 2;
		result->characters[1] = 0x0000;
		result->characters[2] = 0x7FFFFFFF;
	}
	else {
		result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (8 * sizeof(txInteger)), fxCharSetMeasure);
		result->stringCount = 0;
		result->strings = C_NULL;
		result->characters[0] = 8;
		result->characters[1] = 0x0000;
		result->characters[2] = 0x000A;
		result->characters[3] = 0x000B;
		result->characters[4] = 0x000D;
		result->characters[5] = 0x000E;
		result->characters[6] = 0x2028;
		result->characters[7] = 0x2030;
		result->characters[8] = 0x7FFFFFFF;
	}
	return result;
}

void* fxCharSetCanonicalizeSingle(txPatternParser* parser, txCharSet* set)
{
	if ((parser->flags & XS_REGEXP_I) && (set->characters[0] == 2) && (set->characters[1] + 1 == set->characters[2])) {
		txBoolean flag = (parser->flags & XS_REGEXP_UV) ? 1 : 0;
		set->characters[1] = fxCharCaseCanonicalize(set->characters[1], flag);
		set->characters[2] = set->characters[1] + 1;
	}
	return set;
}

void* fxCharSetCombine(txPatternParser* parser, txCharSet* set1, txCharSet* set2, txInteger op)
{
	txInteger flag = 0;
	txInteger old = 0;
	txInteger* current1 = set1->characters + 1;
	txInteger* limit1 = current1 + set1->characters[0];
	txInteger* current2 = set2->characters + 1;
	txInteger* limit2 = current2 + set2->characters[0];
	txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + ((set1->characters[0] + set2->characters[0]) * sizeof(txInteger)), fxCharSetMeasure);
	txInteger count0 = 0;
	txInteger* current0 = result->characters + 1;
	while ((current1 < limit1) && (current2 < limit2)) {
		txInteger test = *current1 - *current2;
		txInteger character;
		if (test <= 0) {
			character = *current1;
			flag ^= 1;
			current1++;
		}
		if (test >= 0) {
			character = *current2;
			flag ^= 2;
			current2++;
		}
		if ((flag == op) || (old == op)) {
			count0++;
			*current0++ = character;
		}
		old = flag;
	}
	if ((op & 2) == 0) {
		while (current1 < limit1) {
			count0++;
			*current0++ = *current1++;
		}
	}
	if ((op & 1) == 0) {
		while (current2 < limit2) {
			count0++;
			*current0++ = *current2++;
		}
	}
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = count0;
	if (set1->strings && set2->strings) {
		txInteger count1 = set1->stringCount;
		txInteger count2 = set2->stringCount;
		txInteger index1 = 0;
		txInteger index2 = 0;
		txInteger count = 0;
		txString string1 = set1->strings[index1];
		txString string2 = set2->strings[index2];
		txInteger length1 = fxUnicodeLength(string1);
		txInteger length2 = fxUnicodeLength(string2);
		if (op == mxCharSetUnionOp)
			count = count1 + count2;
		else if (op == mxCharSetSubtractOp)
			count = count1;
		else
			count = (count1 < count2) ? count1 : count2;
		result->strings = fxPatternParserCreateChunk(parser, count * sizeof(txString));
		count = 0;
		while ((index1 < count1) && (index2 < count2)) {
			txInteger copy = 0;
			txInteger advance = 0;
			if (length1 > length2) {
				if (op != mxCharSetIntersectionOp)
					copy = 1;
				advance = 1;
			}
			else if (length1 < length2) {
				if (op == mxCharSetUnionOp)
					copy = 2;
				advance = 2;
			}
			else {
				txInteger delta = c_strcmp(string1, string2);
				if (delta > 0) {
					if (op != mxCharSetIntersectionOp)
						copy = 1;
					advance = 1;
				}
				else if (delta < 0) {
					if (op == mxCharSetUnionOp)
						copy = 2;
					advance = 2;
				}
				else {
					if (op != mxCharSetSubtractOp)
						copy = 1;
					advance = 3;
				}
			}
			if (copy == 1) {
				result->strings[count] = string1;
				count++;
			}
			else if (copy == 2) {
				result->strings[count] = string2;
				count++;
			}
			if (advance & 1) {
				index1++;
				if (index1 < count1) {
					string1 = set1->strings[index1];
					length1 = fxUnicodeLength(string1);
				}
			}
			if (advance & 2) {
				index2++;
				if (index2 < count2) {
					string2 = set2->strings[index2];
					length2 = fxUnicodeLength(string2);
				}
			}
		}
		if (index1 < count1) {
			if (op != mxCharSetIntersectionOp) {
				while (index1 < count1) {
					result->strings[count] = set1->strings[index1];
					count++;
					index1++;
				}
			}
		}
		else if (index2 < count2) {
			if (op == mxCharSetUnionOp) {
				while (index2 < count2) {
					result->strings[count] = set2->strings[index2];
					count++;
					index2++;
				}
			}
		}
		if (count)
			result->stringCount = count;
		else
			result->strings = C_NULL;
	}
	else if (set1->strings) {
		if (op != mxCharSetIntersectionOp) {
			result->stringCount = set1->stringCount;
			result->strings = set1->strings;
		}
	}
	else if (set2->strings) {
		if (op == mxCharSetUnionOp) {
			result->stringCount = set2->stringCount;
			result->strings = set2->strings;
		}
	}
	return result;
}

void* fxCharSetDigits(txPatternParser* parser)
{
	txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (2 * sizeof(txInteger)), fxCharSetMeasure);
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = 2;
	result->characters[1] = '0';
	result->characters[2] = '9' + 1;
	return result;
}

void* fxCharSetEmpty(txPatternParser* parser)
{
	txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet), fxCharSetMeasure);
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = 0;
	return result;
}

void* fxCharSetExpression(txPatternParser* parser)
{
	txBoolean not = 0;
	void* result = NULL;
	void* left = NULL;
	txInteger leftKind = 0;
	void* right = NULL;
	txInteger rightKind = 0;
	txString string;
	if (parser->character == '^') {
		fxPatternParserNext(parser);
		not = 1;
	}
	left = fxCharSetOperand(parser, &leftKind);
	string = parser->pattern + parser->offset;
	if ((parser->character == '-') && (*string == '-')) {
		for (;;) {
			fxPatternParserNext(parser);
			fxPatternParserNext(parser);
			right = fxCharSetOperand(parser, &rightKind);	
			result = fxCharSetCombine(parser, left, right, mxCharSetSubtractOp);
			if (parser->character == ']')
				break;		
			string = parser->pattern + parser->offset;
			if ((parser->character == '-') && (*string == '-')) {
				left = result;
				continue;
			}
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		}
	}
	else if ((parser->character == '&') && (*string == '&')) {
		for (;;) {
			fxPatternParserNext(parser);
			fxPatternParserNext(parser);
			right = fxCharSetOperand(parser, &rightKind);	
			result = fxCharSetCombine(parser, left, right, mxCharSetIntersectionOp);
			if (parser->character == ']')
				break;		
			string = parser->pattern + parser->offset;
			if ((parser->character == '&') && (*string == '&')) {
				left = result;
				continue;
			}
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		}
	}
	else {
		for (;;) {
			if (parser->character == '-') {
				fxPatternParserNext(parser);
				right = fxCharSetOperand(parser, &rightKind);
				if ((leftKind != 0) && (rightKind != 0))
					fxPatternParserError(parser, gxErrors[mxInvalidRange]);
				left = fxCharSetRange(parser, left, right);
			}
			if (result)
				result = fxCharSetCombine(parser, result, left, mxCharSetUnionOp);
			else
				result = left;
			if (parser->character == ']')
				break;		
			left = fxCharSetOperand(parser, &leftKind);
		}
	}
	if (not && result)
		result = fxCharSetNot(parser, result);
	return result;
}

void* fxCharSetNot(txPatternParser* parser, txCharSet* set)
{
	if (set->strings == C_NULL) {
		txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + ((set->characters[0] + 2) * sizeof(txInteger)), fxCharSetMeasure);
		txInteger* current1 = set->characters + 1;
		txInteger* limit1 = current1 + set->characters[0];
		txInteger count0 = 0;
		txInteger* current0 = result->characters + 1;
		txInteger character = 0;
		while (current1 < limit1) {
			txInteger begin = *current1++;
			txInteger end = *current1++;
			if (character < begin) {
				count0++;
				*current0++ = character;
				count0++;
				*current0++ = begin;
			}
			character = end;
		}
		if (character < 0x7FFFFFFF) {
			count0++;
			*current0++ = character;
			count0++;
			*current0++ = 0x7FFFFFFF;
		}
		result->stringCount = 0;
		result->strings = C_NULL;
		result->characters[0] = count0;
		return result;
	}
	fxPatternParserError(parser, gxErrors[mxInvalidPattern]);
	return C_NULL;
}

void* fxCharSetOperand(txPatternParser* parser, txInteger* kind)
{
	void* result = NULL;
	if (parser->character == EOF) {
		fxPatternParserError(parser, gxErrors[mxInvalidRange]);
	}
	else if (parser->character == '[') {
		fxPatternParserNext(parser);
		result = fxCharSetExpression(parser);	
		*kind = 1;
		if (parser->character != ']')
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		fxPatternParserNext(parser);
	}
	else if (parser->character == '\\') {
		fxPatternParserNext(parser);
		if (parser->character == 'q') {
			result = fxCharSetStrings(parser);
			*kind = 1;
		}
		else
			result = fxCharSetParseEscape(parser, 1, kind);
	}
	else if (c_strchr("&!#$%*+,.:;<=>?@^`~", parser->character)) {
		txInteger character = parser->character;
		fxPatternParserNext(parser);
		if (character == parser->character)
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		result = fxCharSetSingle(parser, character);
		*kind = 0;
	}
	else if (c_strchr("()[]{}/-\\|", parser->character)) {
		fxPatternParserError(parser, gxErrors[mxInvalidRange]);
	}
	else {
		result = fxCharSetSingle(parser, parser->character);
		fxPatternParserNext(parser);
		*kind = 0;
	}
	return result;
}

void* fxCharSetParseEscape(txPatternParser* parser, txBoolean punctuator, txInteger* kind)
{
	void* result = NULL;
	txInteger flag = 1;
	switch(parser->character) {
	case C_EOF:
		break;
	// character classes
	case 'd': 
		result = fxCharSetDigits(parser); 
		fxPatternParserNext(parser);
		break;
	case 'D':
		result = fxCharSetNot(parser, fxCharSetDigits(parser));
		fxPatternParserNext(parser);
		break;
	case 's':
		result = fxCharSetSpaces(parser);
		fxPatternParserNext(parser);
		break;
	case 'S':
		result = fxCharSetNot(parser, fxCharSetSpaces(parser));
		fxPatternParserNext(parser);
		break;
	case 'w':
		result = fxCharSetWords(parser);
		fxPatternParserNext(parser);
		break;
	case 'W':
		result = fxCharSetNot(parser, fxCharSetWords(parser));
		fxPatternParserNext(parser);
		break;
	case 'p':
		result = fxCharSetUnicodeProperty(parser);
		fxPatternParserNext(parser);
		break;
	case 'P':
		result = fxCharSetNot(parser, fxCharSetUnicodeProperty(parser));
		fxPatternParserNext(parser);
		break;
	default:
		fxPatternParserEscape(parser, punctuator);
		result = fxCharSetSingle(parser, parser->character);
		fxPatternParserNext(parser);
		flag = 0;
		break;
	}
	if (result == NULL)
		fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	if (kind)
		*kind = flag;
	return result;
}

void* fxCharSetParseItem(txPatternParser* parser)
{
	void* result = NULL;
	if (parser->character == '-') {
		result = fxCharSetSingle(parser, '-');
		fxPatternParserNext(parser);
	}
	else if (parser->character == '\\') {
		fxPatternParserNext(parser);
		if (parser->character == 'b') {
			fxPatternParserNext(parser);
			result = fxCharSetSingle(parser, 8);
		}
		else if (parser->character == '-') {
			fxPatternParserNext(parser);
			result = fxCharSetSingle(parser, '-');
		}
		else
			result = fxCharSetParseEscape(parser, 0, C_NULL);
	}
	else if (parser->character == ']') {
		result = fxCharSetEmpty(parser);
	}	
	else {
		result = fxCharSetSingle(parser, parser->character);
		fxPatternParserNext(parser);
	}
	return result;
}

void* fxCharSetParseList(txPatternParser* parser)
{
	txBoolean not = 0;
	void* former = NULL;
	void* result = NULL;
	if (parser->character == '^') {
		fxPatternParserNext(parser);
		not = 1;
	}
	while (parser->character != C_EOF) {
		result = fxCharSetParseItem(parser);
		if (parser->character == '-') {
			fxPatternParserNext(parser);
			if (parser->character == ']') {
				result = fxCharSetCanonicalizeSingle(parser, result);
				result = fxCharSetCombine(parser, result, fxCharSetSingle(parser, '-'), mxCharSetUnionOp);
			}
			else {
				result = fxCharSetRange(parser, result, fxCharSetParseItem(parser));
			}
		}
		else
			result = fxCharSetCanonicalizeSingle(parser, result);
		if (former)
			result = fxCharSetCombine(parser, former, result, mxCharSetUnionOp);
		former = result;
		if (parser->character == ']')
			break;
	}
	if (not && result)
		result = fxCharSetNot(parser, result);
	return result;
}

void* fxCharSetRange(txPatternParser* parser, txCharSet* set1, txCharSet* set2)
{
	if ((set1->strings == C_NULL) && (set2->strings == C_NULL)) {
		txCharSet* result;
		if (set1->characters[0] == 0)
			return set2;
		if (set2->characters[0] == 0)
			return set1;
		if ((set1->characters[0] != 2) || (set2->characters[0] != 2))
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		if ((set1->characters[1] + 1 != set1->characters[2]) || (set2->characters[1] + 1 != set2->characters[2]))
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		if ((set1->characters[1] > set2->characters[1]))
			fxPatternParserError(parser, gxErrors[mxInvalidRange]);
		if (parser->flags & XS_REGEXP_I) {
			txBoolean flag = (parser->flags & XS_REGEXP_UV) ? 1 : 0;
			txInteger begin = set1->characters[1];
			txInteger end = set2->characters[1];
			struct {
				txTermPart;
				txInteger stringCount;
				txString* strings;
				txInteger characters[3];
			} _set;
			txCharSet* set = (txCharSet*)&_set;
			set->next = C_NULL;
			set->dispatch.code = fxCharSetCode;
			set->stringCount = 0;
			set->strings = C_NULL;
			set->characters[0] = 2;
			result = fxCharSetEmpty(parser);
			while (begin <= end) {
				set->characters[1] = fxCharCaseCanonicalize(begin, flag);
				set->characters[2] = set->characters[1] + 1;
				result = fxCharSetCombine(parser, result, set, mxCharSetUnionOp);
				begin++;
			}
		}
		else {
			result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (2 * sizeof(txInteger)), fxCharSetMeasure);
			result->stringCount = 0;
			result->strings = C_NULL;
			result->characters[0] = 2;
			result->characters[1] = set1->characters[1];
			result->characters[2] = set2->characters[2];
		}
		return result;
	}
	fxPatternParserError(parser, gxErrors[mxInvalidPattern]);
	return C_NULL;
}

void* fxCharSetSingle(txPatternParser* parser, txInteger character)
{
	txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (2 * sizeof(txInteger)), fxCharSetMeasure);
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = 2;
	result->characters[1] = character;
	result->characters[2] = character + 1;
	return result;
}

void* fxCharSetSpaces(txPatternParser* parser)
{
	txCharSet* result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (20 * sizeof(txInteger)), fxCharSetMeasure);
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = 20;
	result->characters[1] = 0x0009;
	result->characters[2] = 0x000D + 1;
	result->characters[3] = 0x0020;
	result->characters[4] = 0x0020 + 1;
	result->characters[5] = 0x00A0;
	result->characters[6] = 0x00A0 + 1;
	result->characters[7] = 0x1680;
	result->characters[8] = 0x1680 + 1;
	result->characters[9] = 0x2000;
	result->characters[10] = 0x200A + 1;
	result->characters[11] = 0x2028;
	result->characters[12] = 0x2029 + 1;
	result->characters[13] = 0x202F;
	result->characters[14] = 0x202F + 1;
	result->characters[15] = 0x205F;
	result->characters[16] = 0x205F + 1;
	result->characters[17] = 0x3000;
	result->characters[18] = 0x3000 + 1;
	result->characters[19] = 0xFEFF;
	result->characters[20] = 0xFEFF + 1;
	return result;
}

static int fxCharSetStringsCompare(const void *p1, const void *p2)
{
	txString s1 = *((txString*)p1);
	txString s2 = *((txString*)p2);
	txInteger length1 = fxUnicodeLength(s1);
	txInteger length2 = fxUnicodeLength(s2);
	if (length1 > length2) return -1;
	if (length1 < length2) return 1;
	return c_strcmp(s2, s1);
}

void* fxCharSetStrings(txPatternParser* parser)
{
	txCharSet* result = C_NULL;
	txString buffer = C_NULL;
	txInteger bufferSize = 0;
	txString* strings = C_NULL;
	txInteger stringCount = 0;
	txInteger offset, size, length, character;
	txString string;
	
	fxPatternParserNext(parser);
	if (parser->character != '{')
		fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
		
	offset = parser->offset;
	fxPatternParserNext(parser);
	length = 0;	
	for (;;) {
		if (parser->character == EOF)
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);	
		if ((parser->character == '}') || (parser->character == '|')) {
			if (length > 1) {
				bufferSize++;
				stringCount++;
			}
			if (parser->character == '}')
				break;
			length = 0;	
			fxPatternParserNext(parser);
		}
		else {
			if (parser->character == '\\') {
				fxPatternParserNext(parser);
				fxPatternParserEscape(parser, 1);
			}
			character = parser->character;
			if (parser->flags & XS_REGEXP_I)
				character = fxCharCaseCanonicalize(character, 1);
			bufferSize += fxUTF8Length(character); // no surrogate
			length++;
			fxPatternParserNext(parser);
		}		
	}
	
	result = fxPatternParserCreateTerm(parser, sizeof(txCharSet), fxCharSetMeasure);
	result->stringCount = 0;
	result->strings = C_NULL;
	result->characters[0] = 0;
	if (bufferSize > 0) {
		buffer = fxPatternParserCreateChunk(parser, bufferSize);
	}
	if (stringCount > 0) {
		strings = fxPatternParserCreateChunk(parser, stringCount * sizeof(txString));
		stringCount = 0;
	}
	
	parser->offset = offset;
	fxPatternParserNext(parser);
	offset = 0;
	string = buffer;
	length = 0;
	for (;;) {
		if ((parser->character == '}') || (parser->character == '|')) {
			if (length == 0) {
				//??
			}
			else if (length == 1) {
				result = fxCharSetCombine(parser, result, fxCharSetSingle(parser, character), mxCharSetUnionOp);
			}
			else {
				txInteger index = 0;
				buffer[offset] = 0;
				offset++;
				if (stringCount > 0) {
					while (index < stringCount) {
						if (!c_strcmp(strings[index], string))
							break;
						index++;
					}
				}
				if (index == stringCount) {
					strings[stringCount] = string;
					stringCount++;
				}
			}
			if (parser->character == '}') {
				fxPatternParserNext(parser);
				if (strings) {
					if (stringCount > 1)
						qsort(strings, stringCount, sizeof(txString), fxCharSetStringsCompare);
					result->stringCount = stringCount;
					result->strings = strings;
				}
				return result;
			}
			fxPatternParserNext(parser);
			string = buffer + offset;
			length = 0;
		}
		else {
			if (parser->character == '\\') {
				fxPatternParserNext(parser);
				fxPatternParserEscape(parser, 1);
			}
			character = parser->character;
			if (parser->flags & XS_REGEXP_I)
				character = fxCharCaseCanonicalize(character, 1);
			size = fxUTF8Length(character); // no surrogate
			fxUTF8Encode(buffer + offset, character); // no surrogate
			offset += size;
			length++;
			fxPatternParserNext(parser);
		}
	}
}

void* fxCharSetStringsDisjunction(txPatternParser* parser, txCharSet* set)
{
	void* result = set;
	if (set->strings) {
		txDisjunction* disjunction;
		txInteger index = 0;
		void* left = NULL;
		void* right = NULL;
		while (index < set->stringCount) {
			right = fxCharSetStringsSequence(parser, set->strings[index]);
			if (left) {
				disjunction = fxPatternParserCreateTerm(parser, sizeof(txDisjunction), fxDisjunctionMeasure);
				disjunction->left = left;
				disjunction->right = right;
				result = disjunction;
			}
			else {
				result = right;
			}
			left = result;
			index++;
		}
		if (set->characters[0]) {
			disjunction = fxPatternParserCreateTerm(parser, sizeof(txDisjunction), fxDisjunctionMeasure);
			disjunction->left = result;
			disjunction->right = (txTerm*)set;
			result = disjunction;
		}
	}
	return result;
}

void* fxCharSetStringsSequence(txPatternParser* parser, txString string)
{
	void* result = NULL;
	void* former = NULL;
	void* current = NULL;
	txSequence* formerBranch = NULL;
	txSequence* currentBranch = NULL;
	while (*string) {
		txInteger character;
		string = fxUTF8Decode(string, &character); // no surrogate
		current = fxCharSetCanonicalizeSingle(parser, fxCharSetSingle(parser, character));
		if (former) {
			currentBranch = fxPatternParserCreateTerm(parser, sizeof(txSequence), fxSequenceMeasure);
			currentBranch->left = former;
			currentBranch->right = current;
			if (formerBranch)
				formerBranch->right = (txTerm*)currentBranch;
			else
				result = currentBranch;
			formerBranch = currentBranch;	
		}
		else
			result = current;
		former = current;
	}
	return result;
}

void* fxCharSetWords(txPatternParser* parser)
{
	txCharSet* result;
	if (parser->flags & XS_REGEXP_I) {
		if (parser->flags & XS_REGEXP_UV) {
			result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (6 * sizeof(txInteger)), fxCharSetMeasure);
			result->stringCount = 0;
			result->strings = C_NULL;
			result->characters[0] = 6;
			result->characters[1] = '0';
			result->characters[2] = '9' + 1;
			result->characters[3] = '_';
			result->characters[4] = '_' + 1;
			result->characters[5] = 'a';
			result->characters[6] = 'z' + 1;
		}
		else {
			result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (6 * sizeof(txInteger)), fxCharSetMeasure);
			result->stringCount = 0;
			result->strings = C_NULL;
			result->characters[0] = 6;
			result->characters[1] = '0';
			result->characters[2] = '9' + 1;
			result->characters[3] = 'A';
			result->characters[4] = 'Z' + 1;
			result->characters[5] = '_';
			result->characters[6] = '_' + 1;
		}
	}
	else {
		result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + (8 * sizeof(txInteger)), fxCharSetMeasure);
		result->stringCount = 0;
		result->strings = C_NULL;
		result->characters[0] = 8;
		result->characters[1] = '0';
		result->characters[2] = '9' + 1;
		result->characters[3] = 'A';
		result->characters[4] = 'Z' + 1;
		result->characters[5] = '_';
		result->characters[6] = '_' + 1;
		result->characters[7] = 'a';
		result->characters[8] = 'z' + 1;
	}
	return result;
}

void* fxDisjunctionParse(txPatternParser* parser, txInteger character)
{
	txDisjunction* result = NULL;
	txTerm* left = NULL;
	txTerm* right = NULL;
	result = fxSequenceParse(parser, character);
	if (parser->character == '|') {
		fxPatternParserNext(parser);
		left = (txTerm*)result;
		right = fxDisjunctionParse(parser, character);
		result = fxPatternParserCreateTerm(parser, sizeof(txDisjunction), fxDisjunctionMeasure);
		result->left = left;
		result->right = right;
	}
	if (parser->character != character)
		fxPatternParserError(parser, gxErrors[mxInvalidSequence]);
	return result;
}

void* fxQuantifierParse(txPatternParser* parser, void* term, txInteger captureIndex)
{
	txInteger min, max;
	txBoolean greedy;
	txQuantifier* quantifier;
	switch (parser->character) {
	case '*':
		min = 0;
		max = 0x7FFFFFFF;
		fxPatternParserNext(parser);
		break;
	case '+':
		min = 1;
		max = 0x7FFFFFFF;
		fxPatternParserNext(parser);
		break;
	case '?':
		min = 0;
		max = 1;
		fxPatternParserNext(parser);
		break;
	case '{':
		if (fxQuantifierParseBrace(parser, &min, &max)) {
			if (min > max)
				fxPatternParserError(parser, gxErrors[mxInvalidQuantifier]);
			break;
		}
		// continue
	default:
		return term;
	}
	if (parser->character == '?') {
		greedy = 0;
		fxPatternParserNext(parser);
	}
	else
		greedy = 1;
	quantifier = fxPatternParserCreateTerm(parser, sizeof(txQuantifier), fxQuantifierMeasure);
	quantifier->term = term;
	quantifier->min = min;
	quantifier->max = max;
	quantifier->greedy = greedy;
	quantifier->captureIndex = captureIndex;
	quantifier->captureCount = parser->captureIndex - captureIndex;
	quantifier->quantifierIndex = parser->quantifierIndex++;
	return quantifier;
}

txBoolean fxQuantifierParseBrace(txPatternParser* parser, txInteger* min, txInteger* max)
{
	txInteger offset = parser->offset;
	fxPatternParserNext(parser);
	if (!fxQuantifierParseDigits(parser, min))
		goto BACKTRACK;
	if (parser->character == ',') {
		fxPatternParserNext(parser);
		if (parser->character == '}')
			*max = 0x7FFFFFFF;
		else if (!fxQuantifierParseDigits(parser, max))
			goto BACKTRACK;
	}
	else
		*max = *min;
	if (parser->character != '}')
		goto BACKTRACK;
	fxPatternParserNext(parser);
	return 1;
BACKTRACK:
	parser->character = '{';
	parser->offset = offset;
	return 0;
}

txBoolean fxQuantifierParseDigits(txPatternParser* parser, txInteger* result)
{
	txU4 value = 0;
	if (fxPatternParserDecimal(parser, &value)) {
		fxPatternParserNext(parser);
		while (fxPatternParserDecimal(parser, &value))
			fxPatternParserNext(parser);
	}
	else
		return 0;
	if (value > 0x7FFFFFFF)
		value = 0x7FFFFFFF;
	*result = value;
	return 1;
}

void* fxSequenceParse(txPatternParser* parser, txInteger character)
{
	void* result = NULL;
	void* current = NULL;
	txSequence* currentBranch = NULL;
	void* former = NULL;
	txSequence* formerBranch = NULL;
	txInteger length;
	fxPatternParserCheckStack(parser);
	while ((parser->character != C_EOF) && (parser->character != character)) {
		txInteger currentIndex = parser->captureIndex;
		void* term = NULL;
		if (parser->character == '^') {
			fxPatternParserNext(parser);
			current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxLineBeginMeasure);
		}
		else if (parser->character == '$') {
			fxPatternParserNext(parser);
			current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxLineEndMeasure);
		}
		else if (parser->character == '\\') {
			fxPatternParserNext(parser);
			if (parser->character == 'b') {
				fxPatternParserNext(parser);
				current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxWordBreakMeasure);
			}
			else if (parser->character == 'B') {
				fxPatternParserNext(parser);
				current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxWordContinueMeasure);
			}
			else if ((parser->flags & (XS_REGEXP_UV | XS_REGEXP_N)) && (parser->character == 'k')) {
				fxPatternParserNext(parser);
				if (parser->character != '<')
					fxPatternParserError(parser, gxErrors[mxInvalidName]);
				parser->flags |= XS_REGEXP_NAME;
				fxPatternParserNext(parser);
				fxPatternParserName(parser, &length);
				current = fxPatternParserCreateTerm(parser, sizeof(txCaptureReference) + length, fxCaptureReferenceMeasure);
				((txCaptureReference*)current)->captureIndex = -1;
				c_memcpy(((txCaptureReference*)current)->name, parser->error, length + 1);
				current = fxQuantifierParse(parser, current, currentIndex);
			}
			else if (('1' <= parser->character) && (parser->character <= '9')) {
				txU4 value = (txU4)(parser->character - '0');
				fxPatternParserNext(parser);
				while (fxPatternParserDecimal(parser, &value))
					fxPatternParserNext(parser);
				current = fxPatternParserCreateTerm(parser, sizeof(txCaptureReference), fxCaptureReferenceMeasure);
				((txCaptureReference*)current)->captureIndex = value;
				((txCaptureReference*)current)->name[0] = 0;
				current = fxQuantifierParse(parser, current, currentIndex);
			}
			else {
				current = fxCharSetCanonicalizeSingle(parser, fxCharSetParseEscape(parser, 0, C_NULL));
				if (parser->flags & XS_REGEXP_V)
					current = fxCharSetStringsDisjunction(parser, current);
				current = fxQuantifierParse(parser, current, currentIndex);
			}
		}
		else if (parser->character == '.') {
			current = fxCharSetAny(parser);
			fxPatternParserNext(parser);
			current = fxQuantifierParse(parser, current, currentIndex);
		}
		else if (parser->character == '*') {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if (parser->character == '+') {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if (parser->character == '?') {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if (parser->character == '(') {
			fxPatternParserNext(parser);
			if (parser->character == '?') {
				fxPatternParserNext(parser);
				if (parser->character == '=') {
					fxPatternParserNext(parser);
					term = fxDisjunctionParse(parser, ')');
					fxPatternParserNext(parser);
					current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxAssertionMeasure);
					((txAssertion*)current)->term = term;
					((txAssertion*)current)->not = 0;
					((txAssertion*)current)->direction = 1;
					((txAssertion*)current)->assertionIndex = parser->assertionIndex++;
				}
				else if (parser->character == '!') {
					fxPatternParserNext(parser);
					term = fxDisjunctionParse(parser, ')');
					fxPatternParserNext(parser);
					current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxAssertionMeasure);
					((txAssertion*)current)->term = term;
					((txAssertion*)current)->not = 1;
					((txAssertion*)current)->direction = 1;
					((txAssertion*)current)->assertionIndex = parser->assertionIndex++;
				}
				else if (parser->character == ':') {
					fxPatternParserNext(parser);
					current = fxDisjunctionParse(parser, ')');
					fxPatternParserNext(parser);
					current = fxQuantifierParse(parser, current, currentIndex);
				}
				else if (parser->character == '<') {
					parser->flags |= XS_REGEXP_NAME;
					fxPatternParserNext(parser);
					if (parser->character == '=') {
						fxPatternParserNext(parser);
						term = fxDisjunctionParse(parser, ')');
						fxPatternParserNext(parser);
						current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxAssertionMeasure);
						((txAssertion*)current)->term = term;
						((txAssertion*)current)->not = 0;
						((txAssertion*)current)->direction = -1;
						((txAssertion*)current)->assertionIndex = parser->assertionIndex++;
					}
					else if (parser->character == '!') {
						fxPatternParserNext(parser);
						term = fxDisjunctionParse(parser, ')');
						fxPatternParserNext(parser);
						current = fxPatternParserCreateTerm(parser, sizeof(txAssertion), fxAssertionMeasure);
						((txAssertion*)current)->term = term;
						((txAssertion*)current)->not = 1;
						((txAssertion*)current)->direction = -1;
						((txAssertion*)current)->assertionIndex = parser->assertionIndex++;
					}
					else {
						parser->captureIndex++;
						currentIndex++;
						fxPatternParserName(parser, &length);
						current = fxPatternParserCreateTerm(parser, sizeof(txCapture) + length, fxCaptureMeasure);
						((txCapture*)current)->captureIndex = currentIndex;
						c_memcpy(((txCapture*)current)->name, parser->error, length + 1);
						fxPatternParserNamedCapture(parser, (txCapture*)current);
						((txCapture*)current)->term = fxDisjunctionParse(parser, ')');	
						fxPatternParserNext(parser);
						current = fxQuantifierParse(parser, current, currentIndex - 1);
					}
				}
				else {
					fxPatternParserError(parser, gxErrors[mxInvalidGroup]);
				}
			}
			else {
				parser->captureIndex++;
				currentIndex++;
				term = fxDisjunctionParse(parser, ')');
				fxPatternParserNext(parser);
				current = fxPatternParserCreateTerm(parser, sizeof(txCapture), fxCaptureMeasure);
				((txCapture*)current)->term = term;
				((txCapture*)current)->captureIndex = currentIndex;
				((txCapture*)current)->name[0] = 0;
				current = fxQuantifierParse(parser, current, currentIndex - 1);
			}
		}
		else if (parser->character == ')') {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if (parser->character == '[') {
			fxPatternParserNext(parser);
			if (parser->flags & XS_REGEXP_V) {
				current = fxCharSetExpression(parser);
				current = fxCharSetStringsDisjunction(parser, current);
			}
			else
				current = fxCharSetParseList(parser);
			if (parser->character != ']')
				fxPatternParserError(parser, gxErrors[mxInvalidRange]);
			fxPatternParserNext(parser);
			current = fxQuantifierParse(parser, current, currentIndex);
		}
		else if ((parser->character == ']') && (parser->flags & XS_REGEXP_UV)) {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if ((parser->character == '}') && (parser->flags & XS_REGEXP_UV)) {
			fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
		}
		else if (parser->character == '|') {
			break;
		}
		else {
			if (parser->character == '{') {
				txInteger min, max;
				if ((parser->flags & XS_REGEXP_UV))
					fxPatternParserError(parser, gxErrors[mxInvalidCharacter]);
				if (fxQuantifierParseBrace(parser, &min, &max))
					fxPatternParserError(parser, gxErrors[mxInvalidQuantifier]);
			}
			current = fxCharSetCanonicalizeSingle(parser, fxCharSetSingle(parser, parser->character));
			fxPatternParserNext(parser);
			current = fxQuantifierParse(parser, current, currentIndex);
		}
		
		if (former) {
			currentBranch = fxPatternParserCreateTerm(parser, sizeof(txSequence), fxSequenceMeasure);
			currentBranch->left = former;
			currentBranch->right = current;
			if (formerBranch)
				formerBranch->right = (txTerm*)currentBranch;
			else
				result = currentBranch;
			formerBranch = currentBranch;	
		}
		else
			result = current;
		former = current;
	}
	if (result == NULL)
		result = fxPatternParserCreateTerm(parser, sizeof(txTerm), fxEmptyMeasure);
	return result;
}

void* fxTermCreate(txPatternParser* parser, size_t size, txTermMeasure measure)
{
	txTerm* term = c_malloc(size);
	if (!term)
		fxAbort(parser->the, XS_NOT_ENOUGH_MEMORY_EXIT);
	term->next = parser->first;
	term->dispatch.measure = measure;
	parser->first = term;
	return term;
}
	
void fxAssertionMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txAssertion* self = it;
	self->step = parser->size;
	if (self->not)
		parser->size += mxAssertionNotStepSize;
	else		
		parser->size += mxAssertionStepSize;
	(*self->term->dispatch.measure)(parser, self->term, self->direction);
	self->completion = parser->size;
	if (self->not)
		parser->size += mxAssertionNotCompletionSize;
	else		
		parser->size += mxAssertionCompletionSize;
	self->dispatch.code = fxAssertionCode;
}

void fxCaptureMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txCapture* self = it;
	self->step = parser->size;
	parser->size += mxCaptureStepSize;
	(*self->term->dispatch.measure)(parser, self->term, direction);
	self->completion = parser->size;
	parser->size += mxCaptureCompletionSize;
	self->dispatch.code = fxCaptureCode;
}

void fxCaptureReferenceMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txCaptureReference* self = it;
	if (self->captureIndex < 0) {
		txCapture* capture = parser->firstNamedCapture;
		while (capture) {
			if (!c_strcmp(self->name, capture->name))
				break;
			capture = capture->nextNamedCapture;
		}
		if (capture)
			self->captureIndex = capture->captureIndex;
		else
			fxPatternParserError(parser, gxErrors[mxInvalidReferenceName], self->name);
	}
	else if (self->captureIndex >= parser->captureIndex)
		fxPatternParserError(parser, gxErrors[mxInvalidReferenceNumber], self->captureIndex);
	self->step = parser->size;
	parser->size += mxCaptureReferenceStepSize;
	self->dispatch.code = fxCaptureReferenceCode;
}

void fxCharSetMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txCharSet* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize + ((1 + self->characters[0]) * sizeof(txInteger));
	self->dispatch.code = fxCharSetCode;
}

void fxDisjunctionMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txDisjunction* self = it;
	self->step = parser->size;
	parser->size += mxDisjunctionStepSize;
	(*self->left->dispatch.measure)(parser, self->left, direction);
	(*self->right->dispatch.measure)(parser, self->right, direction);
	self->dispatch.code = fxDisjunctionCode;
}
	
void fxEmptyMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txTerm* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize;
	self->dispatch.code = fxEmptyCode;
}

void fxLineBeginMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txTerm* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize;
	self->dispatch.code = fxLineBeginCode;
}

void fxLineEndMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txTerm* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize;
	self->dispatch.code = fxLineEndCode;
}

void fxQuantifierMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txQuantifier* self = it;
	self->step = parser->size;
	parser->size += mxQuantifierStepSize;
	self->loop = parser->size;
	parser->size += mxQuantifierLoopSize;
	(*self->term->dispatch.measure)(parser, self->term, direction);
	self->completion = parser->size;
	parser->size += mxQuantifierCompletionSize;
	self->dispatch.code = fxQuantifierCode;
}

void fxSequenceMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txSequence* self = it;
	fxPatternParserCheckStack(parser);
	if (direction == 1) {
		(*self->left->dispatch.measure)(parser, self->left, direction);
		self->step = self->left->step;
		(*self->right->dispatch.measure)(parser, self->right, direction);
	}
	else {
		(*self->right->dispatch.measure)(parser, self->right, direction);
		self->step = self->right->step;
		(*self->left->dispatch.measure)(parser, self->left, direction);
	}
	self->dispatch.code = fxSequenceCode;
}

void fxWordBreakMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txTerm* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize;
	self->dispatch.code = fxWordBreakCode;
}

void fxWordContinueMeasure(txPatternParser* parser, void* it, txInteger direction)
{
	txTerm* self = it;
	self->step = parser->size;
	parser->size += mxTermStepSize;
	self->dispatch.code = fxWordContinueCode;
}
	
void fxAssertionCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txAssertion* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	if (self->not) {
		*buffer++ = cxAssertionNotStep;
		*buffer++ = self->term->step;
		*buffer++ = self->assertionIndex;
		*buffer++ = sequel;
	}
	else {
		*buffer++ = cxAssertionStep;
		*buffer++ = self->term->step;
		*buffer++ = self->assertionIndex;
	}
	fxPatternParserCheckStack(parser);
	(*self->term->dispatch.code)(parser, self->term, self->direction, self->completion);
	buffer = (txInteger*)(((txByte*)*(parser->code)) + self->completion);
	if (self->not) {
		*buffer++ = cxAssertionNotCompletion;
		*buffer++ = self->assertionIndex;
	}
	else {
		*buffer++ = cxAssertionCompletion;
		*buffer++ = sequel;
		*buffer++ = self->assertionIndex;
	}
}

void fxCaptureCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txCapture* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	if (direction == 1)
		*buffer++ = cxCaptureForwardStep;
	else
		*buffer++ = cxCaptureBackwardStep;
	*buffer++ = self->term->step;
	*buffer++ = self->captureIndex;
	fxPatternParserCheckStack(parser);
	(*self->term->dispatch.code)(parser, self->term, direction, self->completion);
	buffer = (txInteger*)(((txByte*)*(parser->code)) + self->completion);
	if (direction == 1)
		*buffer++ = cxCaptureForwardCompletion;
	else
		*buffer++ = cxCaptureBackwardCompletion;
	*buffer++ = sequel;
	*buffer++ = self->captureIndex;
#ifdef mxRun
	if (parser->the && self->name[0])
		(*parser->code)[2 + self->captureIndex] = fxNewNameC(parser->the, self->name);
	else
#endif
		(*parser->code)[2 + self->captureIndex] = XS_NO_ID;
}

void fxCaptureReferenceCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txCaptureReference* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	if (direction == 1)
		*buffer++ = cxCaptureReferenceForwardStep;
	else
		*buffer++ = cxCaptureReferenceBackwardStep;
	*buffer++ = sequel;
	*buffer++ = self->captureIndex;
}

void fxCharSetCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txCharSet* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	txInteger count, index;
	if (direction == 1)
		*buffer++ = cxCharSetForwardStep;
	else
		*buffer++ = cxCharSetBackwardStep;
	*buffer++ = sequel;
	count = *buffer++ = self->characters[0];
	index = 1;
	while (index <= count) {
		*buffer++ = self->characters[index++];
	}
}

void fxDisjunctionCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txDisjunction* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxDisjunctionStep;
	*buffer++ = self->left->step;
	*buffer++ = self->right->step;
	fxPatternParserCheckStack(parser);
	(*self->left->dispatch.code)(parser, self->left, direction, sequel);
	(*self->right->dispatch.code)(parser, self->right, direction, sequel);
}
	
void fxEmptyCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txTerm* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxEmptyStep;
	*buffer++ = sequel;
}

void fxLineBeginCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txTerm* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxLineBeginStep;
	*buffer++ = sequel;
}

void fxLineEndCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txTerm* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxLineEndStep;
	*buffer++ = sequel;
}

void fxQuantifierCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txQuantifier* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxQuantifierStep;
	*buffer++ = self->loop;
	*buffer++ = self->quantifierIndex;
	*buffer++ = self->min;
	*buffer++ = self->max;
	buffer = (txInteger*)(((txByte*)*(parser->code)) + self->loop);
	if (self->greedy)
		*buffer++ = cxQuantifierGreedyLoop;
	else
		*buffer++ = cxQuantifierLazyLoop;
	*buffer++ = self->term->step;
	*buffer++ = self->quantifierIndex;
	*buffer++ = sequel;
	*buffer++ = self->captureIndex + 1;
	*buffer++ = self->captureIndex + self->captureCount;
	fxPatternParserCheckStack(parser);
	(*self->term->dispatch.code)(parser, self->term, direction, self->completion);
	buffer = (txInteger*)(((txByte*)*(parser->code)) + self->completion);
	*buffer++ = cxQuantifierCompletion;
	*buffer++ = self->loop;
	*buffer++ = self->quantifierIndex;
	*buffer++ = sequel;
}

void fxSequenceCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txSequence* self = it;
	fxPatternParserCheckStack(parser);
	if (direction == 1) {
		(*self->left->dispatch.code)(parser, self->left, direction, self->right->step);
		(*self->right->dispatch.code)(parser, self->right, direction, sequel);
	}
	else {
		(*self->right->dispatch.code)(parser, self->right, direction, self->left->step);
		(*self->left->dispatch.code)(parser, self->left, direction, sequel);
	}
}

void fxWordBreakCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txTerm* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxWordBreakStep;
	*buffer++ = sequel;
}

void fxWordContinueCode(txPatternParser* parser, void* it, txInteger direction, txInteger sequel)
{
	txTerm* self = it;
	txInteger* buffer = (txInteger*)(((txByte*)*(parser->code)) + self->step);
	*buffer++ = cxWordContinueStep;
	*buffer++ = sequel;
}

#if defined(__clang__) || defined (__GNUC__)
	__attribute__((no_sanitize_address))
#endif
void fxPatternParserCheckStack(txPatternParser* parser)
{
    char x;
    char *stack = &x;
    if (stack <= parser->stackLimit)
		fxPatternParserError(parser, gxErrors[mxStackOverflow]);
}

void* fxPatternParserCreateChunk(txPatternParser* parser, txSize size)
{
	txTerm* term = c_malloc(sizeof(txTerm*) + size);
	if (!term)
		fxPatternParserError(parser, gxErrors[mxNotEnoughMemory]);
	term->next = parser->first;
	parser->first = term;
	return (void*)(((txByte*)term) + sizeof(txTerm*));
}

void* fxPatternParserCreateTerm(txPatternParser* parser, size_t size, txTermMeasure measure)
{
	txTerm* term = c_malloc(size);
	if (!term)
		fxPatternParserError(parser, gxErrors[mxNotEnoughMemory]);
	term->next = parser->first;
	term->dispatch.measure = measure;
	parser->first = term;
	return term;
}

void fxPatternParserInitialize(txPatternParser* parser)
{
	c_memset(parser, 0, sizeof(txPatternParser));
}

txBoolean fxPatternParserDecimal(txPatternParser* parser, txU4* value)
{
	txInteger c = parser->character;
	if (('0' <= c) && (c <= '9'))
		*value = (*value * 10) + (c - '0');
	else
		return 0;
	return 1;
}

void fxPatternParserError(txPatternParser* parser, txString format, ...)
{
	c_va_list arguments;
	txString pattern = parser->pattern;
	txString error = parser->error;
	txInteger offset = parser->offset;
    while (offset > 80) {
		txInteger character;
		txString p = mxStringByteDecode(pattern, &character);
    	offset -= (txInteger)(p - pattern);
    	pattern = p;
    }
	while (offset) {
		*error++ = c_read8(pattern++);
		offset--;
	}
	*error++ = ' ';
	c_va_start(arguments, format);
	vsnprintf(error, sizeof(parser->error) - (error - parser->error), format, arguments);
	c_va_end(arguments);
	c_longjmp(parser->jmp_buf, 1);
}

void fxPatternParserEscape(txPatternParser* parser, txBoolean punctuator)
{
	switch(parser->character) {
	case C_EOF:
		break;
	// control escapes
	case 'f':
		parser->character = '\f';
		break;
	case 'n':
		parser->character = '\n';
		break;
	case 'r':
		parser->character = '\r';
		break;
	case 't':
		parser->character = '\t';
		break;
	case 'v':
		parser->character = '\v';
		break;
	// control letters
	case 'c': {
		txInteger value;
		fxPatternParserNext(parser);
		value = parser->character;	
		if ((('a' <= value) && (value <= 'z')) || (('A' <= value) && (value <= 'Z')))
			parser->character = value % 32;
		else
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	} break;
	// null
	case '0': {
		txString p = parser->pattern + parser->offset;
		if ((*p < '0') || ('9' < *p))
			parser->character = 0;
		else
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	} break;
	case 'x': {
		txString p = parser->pattern + parser->offset;
		if (fxParseHexEscape(&p, &parser->character))
			parser->offset = mxPtrDiff(p - parser->pattern);
		else if (parser->flags & XS_REGEXP_UV)
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	} break;
	case 'u': {
		txString p = parser->pattern + parser->offset;
		if (fxParseUnicodeEscape(&p, &parser->character, (parser->flags & XS_REGEXP_UV) ? 1 : 0, (parser->flags & XS_REGEXP_UV) ? '\\' : 0))
			parser->offset = mxPtrDiff(p - parser->pattern);
		else if (parser->flags & XS_REGEXP_UV)
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	} break;
	case '^':
	case '$':
	case '\\':
	case '.':
	case '*':
	case '+':
	case '?':
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case '|':
	case '/':
		break;
	default:
		if (punctuator) {
			switch(parser->character) {
			case 'b':
				parser->character = '\b';
				break;
			case '&':
			case '-':
			case '!':
			case '#':
			case '%':
			case ',':
			case ':':
			case ';':
			case '<':
			case '=':
			case '>':
			case '@':
			case '`':
			case '~':
				break;
			}
		}
		else if (parser->flags & XS_REGEXP_UV)
			fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
		break;
	}
}

void fxPatternParserName(txPatternParser* parser, txInteger* length)
{
	txString p = parser->error;
	txString q = p + sizeof(parser->error) - 1;
	if (parser->character == '\\')
		fxPatternParserNameEscape(parser);
	if (fxIsIdentifierFirst(parser->character)) {
		p = mxStringByteEncode(p, parser->character);
		fxPatternParserNext(parser);
	}
	else
		fxPatternParserError(parser, gxErrors[mxInvalidName]);			
	while (parser->character != '>') {
		if (parser->character == '\\')
			fxPatternParserNameEscape(parser);
		if (fxIsIdentifierNext(parser->character)) {
			if (mxStringByteLength(parser->character) > (q - p))
				fxPatternParserError(parser, gxErrors[mxNameOverflow]);			
			p = mxStringByteEncode(p, parser->character);
			fxPatternParserNext(parser);
		}
		else
			fxPatternParserError(parser, gxErrors[mxInvalidName]);			
	}
	parser->flags &= ~XS_REGEXP_NAME;
	fxPatternParserNext(parser);
	*p = 0;
	*length = mxPtrDiff(p - parser->error);
}

void fxPatternParserNameEscape(txPatternParser* parser)
{
	txString p;
	fxPatternParserNext(parser);
	if (parser->character != 'u')
		fxPatternParserError(parser, gxErrors[mxInvalidName]);			
	p = parser->pattern + parser->offset;
	if (!fxParseUnicodeEscape(&p, &parser->character, 1, '\\'))
		fxPatternParserError(parser, gxErrors[mxInvalidName]);			
	parser->offset = mxPtrDiff(p - parser->pattern);
}

void fxPatternParserNamedCapture(txPatternParser* parser, txCapture* capture)
{
	txCapture* check = parser->firstNamedCapture;
	while (check) {
		if (!strcmp(check->name, capture->name))
			fxPatternParserError(parser, gxErrors[mxDuplicateCapture]);
		check = check->nextNamedCapture;
	}
	capture->nextNamedCapture = parser->firstNamedCapture;
	parser->firstNamedCapture = capture;
}

void fxPatternParserNext(txPatternParser* parser)
{
	txString p = parser->pattern + parser->offset;
	txInteger character;
	
	if (parser->surrogate) {
		parser->character = parser->surrogate;
		parser->surrogate = 0;
	}
	else {
		p = mxStringByteDecode(p, &character);
		if (character != C_EOF) {
			parser->offset = mxPtrDiff(p - parser->pattern);
			if (!(parser->flags & (XS_REGEXP_UV | XS_REGEXP_NAME)) && (character > 0xFFFF)) {
				character -= 0x10000;
				parser->surrogate = 0xDC00 | (character & 0x3FF);
				character = 0xD800 | (character >> 10);
			}
			parser->character = (txInteger)character;
		}
		else
			parser->character = C_EOF;
	}
}

void fxPatternParserTerminate(txPatternParser* parser)
{
	txTerm* term = parser->first;
	while (term) {
		txTerm* next = term->next;
		c_free(term);
		term = next;
	}
	parser->first = NULL;
}

txInteger* fxAllocateRegExpData(void* the, txInteger* code)
{
	txInteger captureCount = code[1];
	txInteger assertionCount = code[2 + captureCount];
	txInteger quantifierCount = code[2 + captureCount + 1];
	txInteger size = captureCount * sizeof(txCaptureData)
					+ assertionCount * sizeof(txAssertionData)
					+ quantifierCount * sizeof(txQuantifierData);
	txInteger* data;
#ifdef mxRun
	if (the)
		data = fxNewChunk(the, size);
#endif
	{
		data = c_malloc(size);
		if (!data)
			fxAbort(the, XS_NOT_ENOUGH_MEMORY_EXIT);
	}
#ifdef mxSnapshot
	c_memset(data, 0, size);
#endif
	return data;
}

txBoolean fxCompileRegExp(void* the, txString pattern, txString modifier, txInteger** code, txInteger** data, txString messageBuffer, txInteger messageSize)
{
	txBoolean result = 1;
	txPatternParser _parser;
	txPatternParser* parser = &_parser;
	txTerm* term;

	fxPatternParserInitialize(parser);
	if (c_setjmp(parser->jmp_buf) == 0) {
		char c;
		while ((c = c_read8(modifier++))) {
			if ((c == 'g') && !(parser->flags & XS_REGEXP_G))
				parser->flags |= XS_REGEXP_G;
			else if ((c == 'i') && !(parser->flags & XS_REGEXP_I))
				parser->flags |= XS_REGEXP_I;
			else if ((c == 'm') && !(parser->flags & XS_REGEXP_M))
				parser->flags |= XS_REGEXP_M;
			else if ((c == 's') && !(parser->flags & XS_REGEXP_S))
				parser->flags |= XS_REGEXP_S;
			else if ((c == 'u') && !(parser->flags & XS_REGEXP_U) && !(parser->flags & XS_REGEXP_V))
				parser->flags |= XS_REGEXP_U;
			else if ((c == 'y') && !(parser->flags & XS_REGEXP_Y))
				parser->flags |= XS_REGEXP_Y;
			else if ((c == 'd') && !(parser->flags & XS_REGEXP_D))
				parser->flags |= XS_REGEXP_D;
			else if ((c == 'v') && !(parser->flags & XS_REGEXP_U) && !(parser->flags & XS_REGEXP_V))
				parser->flags |= XS_REGEXP_V;
			else
				break;
		}
		if (c)
			fxPatternParserError(parser, gxErrors[mxInvalidFlags]);
		parser->pattern = pattern;
		parser->the = the;
		parser->stackLimit = fxCStackLimit();
		fxPatternParserNext(parser);
		term = fxDisjunctionParse(parser, C_EOF);
		if (parser->firstNamedCapture)
			parser->flags |= XS_REGEXP_N;
		if (!(parser->flags & XS_REGEXP_UV) && (parser->flags & XS_REGEXP_N)) {
			fxPatternParserTerminate(parser);
			parser->offset = 0;
			parser->surrogate = 0;
			parser->assertionIndex = 0;
			parser->captureIndex = 0;
			parser->quantifierIndex = 0;
			parser->firstNamedCapture = NULL;
			fxPatternParserNext(parser);
			term = fxDisjunctionParse(parser, C_EOF);
		}	
		parser->captureIndex++;
		if (!term) 
			fxPatternParserError(parser, gxErrors[mxInvalidPattern]);
		parser->size = (4 + parser->captureIndex) * sizeof(txInteger);
		(*term->dispatch.measure)(parser, term, 1);
			
		if (data) {
			txInteger size = parser->captureIndex * sizeof(txCaptureData)
					+ parser->assertionIndex * sizeof(txAssertionData)
					+ parser->quantifierIndex * sizeof(txQuantifierData);
		#ifdef mxRun
			if (the)
				*data = fxNewChunk(the, size);
			else
		#endif
			{
				*data = c_malloc(size);
				if (!*data)
					fxAbort(the, XS_NOT_ENOUGH_MEMORY_EXIT);
			}	
		#ifdef mxSnapshot
			c_memset(*data, 0, size);
		#endif
		}
		if (code) {
			txInteger offset;
			txInteger* buffer;
			offset = parser->size;
			parser->size += sizeof(txInteger);
		#ifdef mxRun
			if (the)
				*code = fxNewChunk(the, parser->size);
			else
		#endif
			{
				*code = c_malloc(parser->size);
				if (!*code)
					fxAbort(the, XS_NOT_ENOUGH_MEMORY_EXIT);
			}
			c_memset(*code, 0, parser->size);
			parser->code = code;
			buffer = *code;
			buffer[0] = parser->flags;
			buffer[1] = parser->captureIndex;
			buffer[2 + parser->captureIndex] = parser->assertionIndex;
			buffer[2 + parser->captureIndex + 1] = parser->quantifierIndex;
			(*term->dispatch.code)(parser, term, 1, offset);
			buffer = (txInteger*)(((txByte*)*code) + offset);
			*buffer = cxMatchStep;
		}
	}
	else {
		if (messageBuffer) {
			c_strncpy(messageBuffer, parser->error, messageSize - 1);
			messageBuffer[messageSize - 1] = 0;
		}
		result = 0;
		
	}
	fxPatternParserTerminate(parser);
	return result;
}

void fxDeleteRegExp(void* the, txInteger* code, txInteger* data)
{
	if (!the) {
		if (code)
			c_free(code);
		if (data)
			c_free(data);
	}
}

// MATCH

txInteger fxFindCharacter(txString input, txInteger offset, txInteger direction, txInteger flags)
{
	txU1* p = (txU1*)input + offset;
	txU1 c;
#if mxCESU8
	if (flags & XS_REGEXP_UV) {
		txInteger character;
		if (direction > 0) {
			p = (txU1*)mxStringByteDecode((txString)p, &character);
		}
		else if (offset > 0) {
			p--;
			while ((c = c_read8(p))) {
				if ((c & 0xC0) != 0x80)
					break;
				p--;
			}
			fxUTF8Decode((txString)p, &character);
			if ((0x0000DC00 <= character) && (character <= 0x0000DFFF)) {
				txU1* q = p - 1;
				while ((c = c_read8(q))) {
					if ((c & 0xC0) != 0x80)
						break;
					q--;
				}
				fxUTF8Decode((txString)q, &character);
				if ((0x0000D800 <= character) && (character <= 0x0000DBFF))
					p = q;
			}
		}
	}
	else
#endif
	{
		p += direction;
		while ((c = c_read8(p))) {
			if ((c & 0xC0) != 0x80)
				break;
			p += direction;
		}
	}
	return mxPtrDiff(p - (txU1*)input);
}

txInteger fxGetCharacter(txString input, txInteger offset, txInteger flags)
{
	txInteger character;
#if mxCESU8
	if (flags & XS_REGEXP_UV)
		mxStringByteDecode(input + offset, &character);
	else
		fxUTF8Decode(input + offset, &character);
#else
	mxStringByteDecode(input + offset, &character);
#endif
	if (flags & XS_REGEXP_I) {
		txBoolean flag = (flags & XS_REGEXP_UV) ? 1 : 0;
		character = fxCharCaseCanonicalize(character, flag);
	}
	return character;
}

txBoolean fxMatchCharacter(txInteger* characters, txInteger character)
{
	txInteger min = 0;
	txInteger max = characters[0] >> 1;
	txInteger* base = characters + 1;
	while (min < max) {
		txInteger mid = (min + max) >> 1;
		txInteger begin = *(base + (mid << 1));
		txInteger end = *(base + (mid << 1) + 1);
		#ifdef mxTrace
			fprintf(stderr, " ");
			if ((32 <= begin) && (begin < 128))
				fprintf(stderr, "%c", begin);
			else
				fprintf(stderr, "%4.4X", begin);
			if (begin != (end - 1)) {
				fprintf(stderr, "-");
				if ((32 <= (end - 1)) && ((end - 1) < 128))
					fprintf(stderr, "%c", (end - 1));
				else
					fprintf(stderr, "%4.4X", (end - 1));
			}
		#endif
		if (character < begin)
			max = mid;
		else if (character >= end)
			min = mid + 1;
		else
			return 1;
	}
	return 0;
}

txStateData* fxPopStates(txMachine* the, txStateData* fromState, txStateData* toState)
{
	txStateData* state = fromState;
	while (state != toState) {
		fromState = state->nextState;
		if (!state->the)
			c_free(state);
		state = fromState;
	}
	return toState;
}

txStateData* fxPushState(txMachine* the, txStateData* firstState, txInteger step, txInteger offset, txCaptureData* captures, txInteger captureCount)
{
	txInteger size = sizeof(txStateData) + ((captureCount - 1) * sizeof(txCaptureData));
	txStateData* state = C_NULL;
	if (the && ((firstState == C_NULL) || (firstState->the != C_NULL))) {
		txByte* current = (txByte*)firstState;
		if (current)
			current += size;
		else
			current = (txByte*)(the->stackBottom);
		if ((current + size) < (txByte*)(the->stack)) {
			state = (txStateData*)current;
			state->the = the;
		}
	}
	if (!state) {
		state = c_malloc(size);
		if (!state)
			fxAbort(the, XS_NOT_ENOUGH_MEMORY_EXIT);
		state->the = C_NULL;
	}	
	state->nextState = firstState;
	state->step = step;
	state->offset = offset;
	c_memcpy(state->captures, captures, captureCount * sizeof(txCaptureData));
	return state;
}

#if defined(__GNUC__)
	#define mxBreak continue
	#define mxCase(WHICH) WHICH
	#define mxSwitch(WHICH) goto *steps[WHICH];
#else
	#define mxBreak \
		break
	#define mxCase(WHICH) \
		case WHICH
	#define mxSwitch(WHICH) \
		switch(WHICH)
#endif

txBoolean fxMatchRegExp(void* the, txInteger* code, txInteger* data, txString subject, txInteger start)
{
#if defined(__GNUC__)
	static void *const gxSteps[] = {
		&&cxMatchStep,
		&&cxAssertionStep,
		&&cxAssertionCompletion,
		&&cxAssertionNotStep,
		&&cxAssertionNotCompletion,
		&&cxCaptureForwardStep,
		&&cxCaptureForwardCompletion,
		&&cxCaptureBackwardStep,
		&&cxCaptureBackwardCompletion,
		&&cxCaptureReferenceForwardStep,
		&&cxCaptureReferenceBackwardStep,
		&&cxCharSetForwardStep,
		&&cxCharSetBackwardStep,
		&&cxDisjunctionStep,
		&&cxEmptyStep,
		&&cxLineBeginStep,
		&&cxLineEndStep,
		&&cxQuantifierStep,
		&&cxQuantifierGreedyLoop,
		&&cxQuantifierLazyLoop,
		&&cxQuantifierCompletion,
		&&cxWordBreakStep,
		&&cxWordContinueStep
	};
	register void * const *steps = gxSteps;
#endif
	txInteger stop = mxStringLength(subject);
	txInteger flags = code[0];
	txCaptureData* captures = (txCaptureData*)data;
	txCaptureData* capture;
	txInteger captureCount = code[1];
	txAssertionData* assertions = (txAssertionData*)(captures + captureCount);
	txAssertionData* assertion;
	txQuantifierData* quantifiers = (txQuantifierData*)(assertions + code[2 + captureCount]);
	txQuantifierData* quantifier;
	txStateData* firstState = C_NULL;
	txInteger from, to, e, f, g;
	txBoolean result = 0;
	if ((0 <= start) && (start <= stop)) {
		for (;;) {
			txInteger step = (2 + captureCount + 2) * sizeof(txInteger), sequel;
			txInteger offset = start;
		#ifdef mxMetering
			txInteger former = step;
		#endif
			c_memset(captures, -1, captureCount * sizeof(txCaptureData));
			while (step) {
				txInteger* pointer = (txInteger*)(((txByte*)code) + step);
				txInteger which = *pointer++;
				#ifdef mxTrace 
				{
					txInteger captureIndex = 0;
					txStateData* state = firstState;
					while (state) {
						captureIndex++;
						state = state->nextState;
					}					
					
					fprintf(stderr, "\n#%d (%d) [%d,%d]", step, captureIndex, start, offset);
					for (captureIndex = 1; captureIndex < captureCount; captureIndex++) 
						fprintf(stderr, " [%d,%d]", captures[captureIndex].from, captures[captureIndex].to);
					fprintf(stderr, " %s", gxStepNames[which]);
				}
				#endif
				#ifdef mxMetering
				if (the) {
					if (step < former) {
						if (((txMachine*)the)->meterInterval && (((txMachine*)the)->meterIndex > ((txMachine*)the)->meterCount)) {
							fxCheckMetering(the);
						}
					}
					former = step;
					((txMachine*)the)->meterIndex++;
				}
				#endif
				mxSwitch(which) {
					mxCase(cxMatchStep):
						capture = captures;
						capture->from = start;
						capture->to = offset;
						step = 0;
						result = 1;
						mxBreak;
					mxCase(cxAssertionStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						assertion = assertions + *pointer;
						assertion->offset = offset;
						assertion->firstState = firstState;
						mxBreak;
					mxCase(cxAssertionCompletion):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						assertion = assertions + *pointer;
						offset = assertion->offset;
						firstState = fxPopStates(the, firstState, assertion->firstState);
						mxBreak;
					mxCase(cxAssertionNotStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						assertion = assertions + *pointer++;
						assertion->offset = offset;
						assertion->firstState = firstState;
						sequel = *pointer;
						firstState = fxPushState(the, firstState, sequel, offset, captures, captureCount);
						if (!firstState)
							return 0;
						mxBreak;
					mxCase(cxAssertionNotCompletion):
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						assertion = assertions + *pointer;
						offset = assertion->offset;
						firstState = fxPopStates(the, firstState, assertion->firstState);
						goto mxPopState;
					mxCase(cxCaptureBackwardStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						capture->to = offset;
						mxBreak;
					mxCase(cxCaptureBackwardCompletion):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						capture->from = offset;
						mxBreak;
					mxCase(cxCaptureForwardStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						capture->from = offset;
						mxBreak;
					mxCase(cxCaptureForwardCompletion):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						capture->to = offset;
						mxBreak;
					mxCase(cxCaptureReferenceBackwardStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						from = capture->from;
						to = capture->to;
						if ((from >= 0) && (to >= 0)) {
							e = offset;
							f = e - (to - from);
							if (f < 0)
								goto mxPopState;
							g = f;
							while (from < to) {
								if (fxGetCharacter(subject, g, flags) != fxGetCharacter(subject, from, flags))
									goto mxPopState;
								g = fxFindCharacter(subject, g, 1, flags);
								from = fxFindCharacter(subject, from, 1, flags);
							}
							offset = f;
						}
						mxBreak;
					mxCase(cxCaptureReferenceForwardStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						capture = captures + *pointer;
						from = capture->from;
						to = capture->to;
						if ((from >= 0) && (to >= 0)) {
							e = offset;
							f = e + (to - from);
							if (f > stop)
								goto mxPopState;
							g = e;
							while (from < to) {
								if (fxGetCharacter(subject, g, flags) != fxGetCharacter(subject, from, flags))
									goto mxPopState;
								g = fxFindCharacter(subject, g, 1, flags);
								from = fxFindCharacter(subject, from, 1, flags);
							}
							offset = f;
						}
						mxBreak;
					mxCase(cxCharSetBackwardStep):
						step = *pointer++;
						e = offset;
						if (e == 0)
							goto mxPopState;
						e = fxFindCharacter(subject, e, -1, flags);
						if (!fxMatchCharacter(pointer, fxGetCharacter(subject, e, flags)))
							goto mxPopState;
						offset = e;
						mxBreak;
					mxCase(cxCharSetForwardStep):
						step = *pointer++;
						e = offset;
						if (e == stop)
							goto mxPopState;
						if (!fxMatchCharacter(pointer, fxGetCharacter(subject, e, flags)))
							goto mxPopState;
						e = fxFindCharacter(subject, e, 1, flags);
						offset = e;
						mxBreak;
					mxCase(cxDisjunctionStep):
						step = *pointer++;
						sequel = *pointer;
						firstState = fxPushState(the, firstState, sequel, offset, captures, captureCount);
						if (!firstState)
							return 0;
						mxBreak;
					mxCase(cxEmptyStep):
						step = *pointer;
						if ((offset != stop) && firstState)
							goto mxPopState;
						mxBreak;
					mxCase(cxLineBeginStep):
						step = *pointer;
						if (offset == 0)
							mxBreak;
						if ((flags & XS_REGEXP_M) && fxMatchCharacter((txInteger*)gxLineCharacters, fxGetCharacter(subject, fxFindCharacter(subject, offset, -1, flags), flags)))
							mxBreak;
						goto mxPopState;
					mxCase(cxLineEndStep):
						step = *pointer;
						if (offset == stop)
							mxBreak;
						if ((flags & XS_REGEXP_M) && fxMatchCharacter((txInteger*)gxLineCharacters, fxGetCharacter(subject, offset, flags)))
							mxBreak;
						goto mxPopState;
					mxCase(cxQuantifierStep):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						quantifier = quantifiers + *pointer++;
						quantifier->min = *pointer++;
						quantifier->max = *pointer;
						quantifier->offset = offset;
						mxBreak;
					mxCase(cxQuantifierGreedyLoop):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						quantifier = quantifiers + *pointer++;
						sequel = *pointer++;
						from = *pointer++;
						to = *pointer;
						#ifdef mxTrace 
							fprintf(stderr, " min=%d", quantifier->min);
							if (quantifier->max != 0x7FFFFFFF)
								fprintf(stderr, " max=%d", quantifier->max);
							fprintf(stderr, " from=%d", from);
							fprintf(stderr, " to=%d", to);
							fprintf(stderr, " offset=%d", quantifier->offset);
						#endif
						if (quantifier->max == 0) {
							step = sequel;
							mxBreak;
						}
						else {
							if (quantifier->min == 0) {
								firstState = fxPushState(the, firstState, sequel, offset, captures, captureCount);
								if (!firstState)
									return 0;
							}
							if (from < to)
								c_memset(captures + from, -1, (to - from) * sizeof(txCaptureData));
						}
						mxBreak;
					mxCase(cxQuantifierLazyLoop):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						quantifier = quantifiers + *pointer++;
						sequel = *pointer++;
						from = *pointer++;
						to = *pointer;
						#ifdef mxTrace 
							fprintf(stderr, " min=%d", quantifier->min);
							if (quantifier->max != 0x7FFFFFFF)
								fprintf(stderr, " max=%d", quantifier->max);
							fprintf(stderr, " from=%d", from);
							fprintf(stderr, " to=%d", to);
							fprintf(stderr, " offset=%d", quantifier->offset);
						#endif
						if (quantifier->max == 0) {
							step = sequel;
							mxBreak;
						}
						if (quantifier->min == 0) {
							firstState = fxPushState(the, firstState, step, offset, captures, captureCount);
							if (!firstState)
								return 0;
							if (from < to)
								c_memset(captures + from, -1, (to - from) * sizeof(txCaptureData));
							step = sequel;
						}
						else {
							if (from < to)
								c_memset(captures + from, -1, (to - from) * sizeof(txCaptureData));
						}
						mxBreak;
					mxCase(cxQuantifierCompletion):
						step = *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " #%d", *pointer);
						#endif
						quantifier = quantifiers + *pointer++;
						#ifdef mxTrace 
							fprintf(stderr, " min=%d", quantifier->min);
							if (quantifier->max != 0x7FFFFFFF)
								fprintf(stderr, " max=%d", quantifier->max);
							fprintf(stderr, " offset=%d", quantifier->offset);
						#endif
						sequel = *pointer;
						if ((quantifier->min == 0) && (quantifier->offset == offset)) {
							step = sequel;
							mxBreak;
						}
						quantifier->min = (quantifier->min == 0) ? 0 : quantifier->min - 1;
						quantifier->max = (quantifier->max == 0x7FFFFFFF) ? 0x7FFFFFFF : (quantifier->max == 0) ? 0 : quantifier->max - 1;
						quantifier->offset = offset;
						mxBreak;
					mxCase(cxWordBreakStep):
						step = *pointer;
						e = (offset == 0) ? 0 : fxMatchCharacter((txInteger*)gxWordCharacters, fxGetCharacter(subject, fxFindCharacter(subject, offset, -1, flags), flags));
						f = (offset == stop) ? 0 : fxMatchCharacter((txInteger*)gxWordCharacters, fxGetCharacter(subject, offset, flags));
						if (e != f)
							mxBreak;
						goto mxPopState;
					mxCase(cxWordContinueStep):
						step = *pointer;
						e = (offset == 0) ? 0 : fxMatchCharacter((txInteger*)gxWordCharacters, fxGetCharacter(subject, fxFindCharacter(subject, offset, -1, flags), flags));
						f = (offset == stop) ? 0 : fxMatchCharacter((txInteger*)gxWordCharacters, fxGetCharacter(subject, offset, flags));
						if (e == f)
							mxBreak;
						goto mxPopState;
				
					mxPopState:
						if (!firstState) {
							step = 0;
							mxBreak;
						}
						#ifdef mxTrace 
							fprintf(stderr, " <<<");
						#endif
						step = firstState->step;
						offset = firstState->offset;
						c_memcpy(captures, firstState->captures, captureCount * sizeof(txCaptureData));
						if (firstState->the) {
							firstState = firstState->nextState;
						}
						else {
							txStateData* state = firstState;
							firstState = state->nextState;
							c_free(state);
						}
						mxBreak;
				}
			}
			#ifdef mxTrace
				fprintf(stderr, "\n###\n");
			#endif
			firstState = fxPopStates(the, firstState, C_NULL);
			if (flags & XS_REGEXP_Y)
				break;
			if (result)
				break;
			if (start == stop)
				break;
			start = fxFindCharacter(subject, start, 1, flags);
		}
	}
	return result;
}

#ifdef mxRegExpUnicodePropertyEscapes

// character sets generated thanks to https://github.com/mathiasbynens/unicode-property-escapes-tests

typedef struct {
	char* id;
	txInteger length;
	const txInteger* data;
} txCharSetUnicodeProperty;

typedef struct {
	char* id;
	txInteger length;
	const txInteger* data;
	txInteger stringCount;
	const txString* strings;
} txCharSetUnicodeStringProperty;

#define mxCharSet_Binary_Property_ASCII 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_ASCII[mxCharSet_Binary_Property_ASCII] = {
	0x000000, 0x000080, 
};
#define mxCharSet_Binary_Property_ASCII_Hex_Digit 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_ASCII_Hex_Digit[mxCharSet_Binary_Property_ASCII_Hex_Digit] = {
	0x000030, 0x00003a, 0x000041, 0x000047, 0x000061, 0x000067, 
};
#define mxCharSet_Binary_Property_Alphabetic 1464
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Alphabetic[mxCharSet_Binary_Property_Alphabetic] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 
	0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000345, 0x000346, 0x000370, 0x000375, 0x000376, 0x000378, 0x00037a, 0x00037e, 
	0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 0x00048a, 0x000530, 
	0x000531, 0x000557, 0x000559, 0x00055a, 0x000560, 0x000589, 0x0005b0, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 
	0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000610, 0x00061b, 0x000620, 0x000658, 0x000659, 0x000660, 0x00066e, 0x0006d4, 0x0006d5, 0x0006dd, 0x0006e1, 0x0006e9, 
	0x0006ed, 0x0006f0, 0x0006fa, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x000740, 0x00074d, 0x0007b2, 0x0007ca, 0x0007eb, 0x0007f4, 0x0007f6, 0x0007fa, 0x0007fb, 
	0x000800, 0x000818, 0x00081a, 0x00082d, 0x000840, 0x000859, 0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x0008a0, 0x0008ca, 0x0008d4, 0x0008e0, 
	0x0008e3, 0x0008ea, 0x0008f0, 0x00093c, 0x00093d, 0x00094d, 0x00094e, 0x000951, 0x000955, 0x000964, 0x000971, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 
	0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009c5, 0x0009c7, 0x0009c9, 0x0009cb, 0x0009cd, 0x0009ce, 0x0009cf, 
	0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009f0, 0x0009f2, 0x0009fc, 0x0009fd, 0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 
	0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4d, 
	0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a70, 0x000a76, 0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 
	0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000acd, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 
	0x000af9, 0x000afd, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 
	0x000b3d, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4d, 0x000b56, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b71, 0x000b72, 0x000b82, 0x000b84, 
	0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 
	0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bcd, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 
	0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3d, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4d, 0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 
	0x000c60, 0x000c64, 0x000c80, 0x000c84, 0x000c85, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cc5, 
	0x000cc6, 0x000cc9, 0x000cca, 0x000ccd, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 0x000d0e, 0x000d11, 
	0x000d12, 0x000d3b, 0x000d3d, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d4d, 0x000d4e, 0x000d4f, 0x000d54, 0x000d58, 0x000d5f, 0x000d64, 0x000d7a, 0x000d80, 
	0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dcf, 0x000dd5, 0x000dd6, 0x000dd7, 
	0x000dd8, 0x000de0, 0x000df2, 0x000df4, 0x000e01, 0x000e3b, 0x000e40, 0x000e47, 0x000e4d, 0x000e4e, 0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 
	0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eba, 0x000ebb, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ecd, 0x000ece, 0x000edc, 0x000ee0, 
	0x000f00, 0x000f01, 0x000f40, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f84, 0x000f88, 0x000f98, 0x000f99, 0x000fbd, 0x001000, 0x001037, 0x001038, 0x001039, 
	0x00103b, 0x001040, 0x001050, 0x001090, 0x00109a, 0x00109e, 0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 
	0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 
	0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001380, 0x001390, 
	0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001401, 0x00166d, 0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 0x001700, 0x001714, 
	0x00171f, 0x001734, 0x001740, 0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017b4, 0x0017b6, 0x0017c9, 0x0017d7, 0x0017d8, 
	0x0017dc, 0x0017dd, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x001939, 0x001950, 0x00196e, 
	0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x001a00, 0x001a1c, 0x001a20, 0x001a5f, 0x001a61, 0x001a75, 0x001aa7, 0x001aa8, 0x001abf, 0x001ac1, 
	0x001acc, 0x001acf, 0x001b00, 0x001b34, 0x001b35, 0x001b44, 0x001b45, 0x001b4d, 0x001b80, 0x001baa, 0x001bac, 0x001bb0, 0x001bba, 0x001be6, 0x001be7, 0x001bf2, 
	0x001c00, 0x001c37, 0x001c4d, 0x001c50, 0x001c5a, 0x001c7e, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 
	0x001cf5, 0x001cf7, 0x001cfa, 0x001cfb, 0x001d00, 0x001dc0, 0x001de7, 0x001df5, 0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 
	0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 
	0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x002071, 0x002072, 
	0x00207f, 0x002080, 0x002090, 0x00209d, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 
	0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002160, 0x002189, 
	0x0024b6, 0x0024ea, 0x002c00, 0x002ce5, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 
	0x002d6f, 0x002d70, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 
	0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x002de0, 0x002e00, 0x002e2f, 0x002e30, 0x003005, 0x003008, 0x003021, 0x00302a, 0x003031, 0x003036, 0x003038, 0x00303d, 
	0x003041, 0x003097, 0x00309d, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 
	0x003400, 0x004dc0, 0x004e00, 0x00a48d, 0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a620, 0x00a62a, 0x00a62c, 0x00a640, 0x00a66f, 0x00a674, 0x00a67c, 
	0x00a67f, 0x00a6f0, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a806, 
	0x00a807, 0x00a828, 0x00a840, 0x00a874, 0x00a880, 0x00a8c4, 0x00a8c5, 0x00a8c6, 0x00a8f2, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a900, 0x00a90a, 0x00a92b, 
	0x00a930, 0x00a953, 0x00a960, 0x00a97d, 0x00a980, 0x00a9b3, 0x00a9b4, 0x00a9c0, 0x00a9cf, 0x00a9d0, 0x00a9e0, 0x00a9f0, 0x00a9fa, 0x00a9ff, 0x00aa00, 0x00aa37, 
	0x00aa40, 0x00aa4e, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aabf, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 0x00aadb, 0x00aade, 0x00aae0, 0x00aaf0, 0x00aaf2, 0x00aaf6, 
	0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abeb, 
	0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb29, 
	0x00fb2a, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 0x00fd50, 0x00fd90, 
	0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfc, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ffbf, 0x00ffc2, 0x00ffc8, 
	0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 
	0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010140, 0x010175, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x010300, 0x010320, 0x01032d, 0x01034b, 0x010350, 0x01037b, 
	0x010380, 0x01039e, 0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x0103d1, 0x0103d6, 0x010400, 0x01049e, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 
	0x010530, 0x010564, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 
	0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 
	0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 
	0x0108f4, 0x0108f6, 0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 0x010a00, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 
	0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a60, 0x010a7d, 0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae5, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 
	0x010b60, 0x010b73, 0x010b80, 0x010b92, 0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d28, 0x010e80, 0x010eaa, 0x010eab, 0x010ead, 
	0x010eb0, 0x010eb2, 0x010f00, 0x010f1d, 0x010f27, 0x010f28, 0x010f30, 0x010f46, 0x010f70, 0x010f82, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011000, 0x011046, 
	0x011071, 0x011076, 0x011080, 0x0110b9, 0x0110c2, 0x0110c3, 0x0110d0, 0x0110e9, 0x011100, 0x011133, 0x011144, 0x011148, 0x011150, 0x011173, 0x011176, 0x011177, 
	0x011180, 0x0111c0, 0x0111c1, 0x0111c5, 0x0111ce, 0x0111d0, 0x0111da, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x011235, 0x011237, 0x011238, 
	0x01123e, 0x011242, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112e9, 0x011300, 0x011304, 
	0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133d, 0x011345, 0x011347, 0x011349, 
	0x01134b, 0x01134d, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011400, 0x011442, 0x011443, 0x011446, 0x011447, 0x01144b, 0x01145f, 0x011462, 
	0x011480, 0x0114c2, 0x0114c4, 0x0114c6, 0x0114c7, 0x0114c8, 0x011580, 0x0115b6, 0x0115b8, 0x0115bf, 0x0115d8, 0x0115de, 0x011600, 0x01163f, 0x011640, 0x011641, 
	0x011644, 0x011645, 0x011680, 0x0116b6, 0x0116b8, 0x0116b9, 0x011700, 0x01171b, 0x01171d, 0x01172b, 0x011740, 0x011747, 0x011800, 0x011839, 0x0118a0, 0x0118e0, 
	0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x01193d, 0x01193f, 0x011943, 
	0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 0x0119e0, 0x0119e1, 0x0119e2, 0x0119e3, 0x0119e5, 0x011a00, 0x011a33, 0x011a35, 0x011a3f, 0x011a50, 0x011a98, 
	0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c3f, 0x011c40, 0x011c41, 0x011c72, 0x011c90, 0x011c92, 0x011ca8, 
	0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d42, 0x011d43, 0x011d44, 
	0x011d46, 0x011d48, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d97, 0x011d98, 0x011d99, 0x011ee0, 0x011ef7, 
	0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f41, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 0x012400, 0x01246f, 0x012480, 0x012544, 0x012f90, 0x012ff1, 
	0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a70, 0x016abf, 0x016ad0, 0x016aee, 0x016b00, 0x016b30, 
	0x016b40, 0x016b44, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e80, 0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 0x016fe0, 0x016fe2, 
	0x016fe3, 0x016fe4, 0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 
	0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 
	0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9e, 0x01bc9f, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 
	0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 
	0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 
	0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 
	0x01d7c4, 0x01d7cc, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 
	0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e137, 0x01e13e, 0x01e14e, 0x01e14f, 0x01e290, 0x01e2ae, 0x01e2c0, 0x01e2ec, 0x01e4d0, 0x01e4ec, 
	0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e900, 0x01e944, 0x01e947, 0x01e948, 0x01e94b, 0x01e94c, 
	0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 
	0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 
	0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 
	0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 
	0x01eeab, 0x01eebc, 0x01f130, 0x01f14a, 0x01f150, 0x01f16a, 0x01f170, 0x01f18a, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 
	0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Binary_Property_Any 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Any[mxCharSet_Binary_Property_Any] = {
	0x000000, 0x00dc00, 0x00dc00, 0x00e000, 0x00e000, 0x110000, 
};
#define mxCharSet_Binary_Property_Assigned 1418
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Assigned[mxCharSet_Binary_Property_Assigned] = {
	0x000000, 0x000378, 0x00037a, 0x000380, 0x000384, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x000530, 0x000531, 0x000557, 0x000559, 0x00058b, 
	0x00058d, 0x000590, 0x000591, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x000600, 0x00070e, 0x00070f, 0x00074b, 0x00074d, 0x0007b2, 0x0007c0, 0x0007fb, 
	0x0007fd, 0x00082e, 0x000830, 0x00083f, 0x000840, 0x00085c, 0x00085e, 0x00085f, 0x000860, 0x00086b, 0x000870, 0x00088f, 0x000890, 0x000892, 0x000898, 0x000984, 
	0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 0x0009c7, 0x0009c9, 
	0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009ff, 0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 
	0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 
	0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a77, 0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 
	0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 
	0x000ae0, 0x000ae4, 0x000ae6, 0x000af2, 0x000af9, 0x000b00, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 
	0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 
	0x000b66, 0x000b78, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 
	0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 
	0x000be6, 0x000bfb, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 
	0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 0x000c66, 0x000c70, 0x000c77, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 
	0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 
	0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d50, 0x000d54, 0x000d64, 
	0x000d66, 0x000d80, 0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 
	0x000dcf, 0x000dd5, 0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df5, 0x000e01, 0x000e3b, 0x000e3f, 0x000e5c, 0x000e81, 0x000e83, 
	0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 
	0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f98, 0x000f99, 0x000fbd, 0x000fbe, 0x000fcd, 0x000fce, 0x000fdb, 
	0x001000, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 
	0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 
	0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x00135d, 0x00137d, 0x001380, 0x00139a, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001400, 0x00169d, 
	0x0016a0, 0x0016f9, 0x001700, 0x001716, 0x00171f, 0x001737, 0x001740, 0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017de, 
	0x0017e0, 0x0017ea, 0x0017f0, 0x0017fa, 0x001800, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 
	0x001930, 0x00193c, 0x001940, 0x001941, 0x001944, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x001a1c, 
	0x001a1e, 0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 0x001ab0, 0x001acf, 0x001b00, 0x001b4d, 0x001b50, 0x001b7f, 
	0x001b80, 0x001bf4, 0x001bfc, 0x001c38, 0x001c3b, 0x001c4a, 0x001c4d, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc8, 0x001cd0, 0x001cfb, 0x001d00, 0x001f16, 
	0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 
	0x001f80, 0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff6, 0x001fff, 0x002000, 0x002065, 
	0x002066, 0x002072, 0x002074, 0x00208f, 0x002090, 0x00209d, 0x0020a0, 0x0020c1, 0x0020d0, 0x0020f1, 0x002100, 0x00218c, 0x002190, 0x002427, 0x002440, 0x00244b, 
	0x002460, 0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002cf4, 0x002cf9, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d71, 
	0x002d7f, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 
	0x002dd8, 0x002ddf, 0x002de0, 0x002e5e, 0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x002ff0, 0x002ffc, 0x003000, 0x003040, 0x003041, 0x003097, 
	0x003099, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x003190, 0x0031e4, 0x0031f0, 0x00321f, 0x003220, 0x00a48d, 0x00a490, 0x00a4c7, 0x00a4d0, 0x00a62c, 
	0x00a640, 0x00a6f8, 0x00a700, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a82d, 0x00a830, 0x00a83a, 0x00a840, 0x00a878, 
	0x00a880, 0x00a8c6, 0x00a8ce, 0x00a8da, 0x00a8e0, 0x00a954, 0x00a95f, 0x00a97d, 0x00a980, 0x00a9ce, 0x00a9cf, 0x00a9da, 0x00a9de, 0x00a9ff, 0x00aa00, 0x00aa37, 
	0x00aa40, 0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aac3, 0x00aadb, 0x00aaf7, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 
	0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab6c, 0x00ab70, 0x00abee, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00d800, 0x00dc00, 
	0x00dc00, 0x00e000, 0x00e000, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 
	0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbc3, 0x00fbd3, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe1a, 0x00fe20, 0x00fe53, 
	0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00feff, 0x00ff00, 0x00ff01, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 
	0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fff9, 0x00fffe, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 
	0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x01018f, 0x010190, 0x01019d, 
	0x0101a0, 0x0101a1, 0x0101d0, 0x0101fe, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x0102e0, 0x0102fc, 0x010300, 0x010324, 0x01032d, 0x01034b, 0x010350, 0x01037b, 
	0x010380, 0x01039e, 0x01039f, 0x0103c4, 0x0103c8, 0x0103d6, 0x010400, 0x01049e, 0x0104a0, 0x0104aa, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 
	0x010530, 0x010564, 0x01056f, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 
	0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 
	0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010857, 0x01089f, 0x0108a7, 0x0108b0, 0x0108e0, 0x0108f3, 
	0x0108f4, 0x0108f6, 0x0108fb, 0x01091c, 0x01091f, 0x01093a, 0x01093f, 0x010940, 0x010980, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a04, 0x010a05, 0x010a07, 
	0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a49, 0x010a50, 0x010a59, 0x010a60, 0x010aa0, 0x010ac0, 0x010ae7, 
	0x010aeb, 0x010af7, 0x010b00, 0x010b36, 0x010b39, 0x010b56, 0x010b58, 0x010b73, 0x010b78, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 0x010c00, 0x010c49, 
	0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010cfa, 0x010d28, 0x010d30, 0x010d3a, 0x010e60, 0x010e7f, 0x010e80, 0x010eaa, 0x010eab, 0x010eae, 0x010eb0, 0x010eb2, 
	0x010efd, 0x010f28, 0x010f30, 0x010f5a, 0x010f70, 0x010f8a, 0x010fb0, 0x010fcc, 0x010fe0, 0x010ff7, 0x011000, 0x01104e, 0x011052, 0x011076, 0x01107f, 0x0110c3, 
	0x0110cd, 0x0110ce, 0x0110d0, 0x0110e9, 0x0110f0, 0x0110fa, 0x011100, 0x011135, 0x011136, 0x011148, 0x011150, 0x011177, 0x011180, 0x0111e0, 0x0111e1, 0x0111f5, 
	0x011200, 0x011212, 0x011213, 0x011242, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 0x0112b0, 0x0112eb, 
	0x0112f0, 0x0112fa, 0x011300, 0x011304, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 
	0x01133b, 0x011345, 0x011347, 0x011349, 0x01134b, 0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 
	0x011400, 0x01145c, 0x01145d, 0x011462, 0x011480, 0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115b6, 0x0115b8, 0x0115de, 0x011600, 0x011645, 0x011650, 0x01165a, 
	0x011660, 0x01166d, 0x011680, 0x0116ba, 0x0116c0, 0x0116ca, 0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x011747, 0x011800, 0x01183c, 0x0118a0, 0x0118f3, 
	0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011947, 0x011950, 0x01195a, 
	0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 0x0119e5, 0x011a00, 0x011a48, 0x011a50, 0x011aa3, 0x011ab0, 0x011af9, 0x011b00, 0x011b0a, 0x011c00, 0x011c09, 
	0x011c0a, 0x011c37, 0x011c38, 0x011c46, 0x011c50, 0x011c6d, 0x011c70, 0x011c90, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 
	0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 
	0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 0x011daa, 0x011ee0, 0x011ef9, 0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f5a, 0x011fb0, 0x011fb1, 
	0x011fc0, 0x011ff2, 0x011fff, 0x01239a, 0x012400, 0x01246f, 0x012470, 0x012475, 0x012480, 0x012544, 0x012f90, 0x012ff3, 0x013000, 0x013456, 0x014400, 0x014647, 
	0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a6e, 0x016abf, 0x016ac0, 0x016aca, 0x016ad0, 0x016aee, 0x016af0, 0x016af6, 0x016b00, 0x016b46, 
	0x016b50, 0x016b5a, 0x016b5b, 0x016b62, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e9b, 0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 
	0x016fe0, 0x016fe5, 0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 
	0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 
	0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9c, 0x01bca4, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 
	0x01d129, 0x01d1eb, 0x01d200, 0x01d246, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d300, 0x01d357, 0x01d360, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 
	0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 
	0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 
	0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01da8c, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 0x01e007, 
	0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 
	0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 0x01e290, 0x01e2af, 0x01e2c0, 0x01e2fa, 0x01e2ff, 0x01e300, 0x01e4d0, 0x01e4fa, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 
	0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e8c7, 0x01e8d7, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 0x01ec71, 0x01ecb5, 
	0x01ed01, 0x01ed3e, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 
	0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 
	0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 
	0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 
	0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 0x01f0c1, 0x01f0d0, 
	0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 0x01f6d8, 
	0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 0x01f810, 0x01f848, 
	0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 
	0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 0x01fbf0, 0x01fbfa, 
	0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
	0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 0x0e0100, 0x0e01f0, 0x0f0000, 0x0ffffe, 0x100000, 0x10fffe, 
};
#define mxCharSet_Binary_Property_Bidi_Control 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Bidi_Control[mxCharSet_Binary_Property_Bidi_Control] = {
	0x00061c, 0x00061d, 0x00200e, 0x002010, 0x00202a, 0x00202f, 0x002066, 0x00206a, 
};
#define mxCharSet_Binary_Property_Bidi_Mirrored 228
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Bidi_Mirrored[mxCharSet_Binary_Property_Bidi_Mirrored] = {
	0x000028, 0x00002a, 0x00003c, 0x00003d, 0x00003e, 0x00003f, 0x00005b, 0x00005c, 0x00005d, 0x00005e, 0x00007b, 0x00007c, 0x00007d, 0x00007e, 0x0000ab, 0x0000ac, 
	0x0000bb, 0x0000bc, 0x000f3a, 0x000f3e, 0x00169b, 0x00169d, 0x002039, 0x00203b, 0x002045, 0x002047, 0x00207d, 0x00207f, 0x00208d, 0x00208f, 0x002140, 0x002141, 
	0x002201, 0x002205, 0x002208, 0x00220e, 0x002211, 0x002212, 0x002215, 0x002217, 0x00221a, 0x00221e, 0x00221f, 0x002223, 0x002224, 0x002225, 0x002226, 0x002227, 
	0x00222b, 0x002234, 0x002239, 0x00223a, 0x00223b, 0x00224d, 0x002252, 0x002256, 0x00225f, 0x002261, 0x002262, 0x002263, 0x002264, 0x00226c, 0x00226e, 0x00228d, 
	0x00228f, 0x002293, 0x002298, 0x002299, 0x0022a2, 0x0022a4, 0x0022a6, 0x0022b9, 0x0022be, 0x0022c0, 0x0022c9, 0x0022ce, 0x0022d0, 0x0022d2, 0x0022d6, 0x0022ee, 
	0x0022f0, 0x002300, 0x002308, 0x00230c, 0x002320, 0x002322, 0x002329, 0x00232b, 0x002768, 0x002776, 0x0027c0, 0x0027c1, 0x0027c3, 0x0027c7, 0x0027c8, 0x0027ca, 
	0x0027cb, 0x0027ce, 0x0027d3, 0x0027d7, 0x0027dc, 0x0027df, 0x0027e2, 0x0027f0, 0x002983, 0x002999, 0x00299b, 0x0029a1, 0x0029a2, 0x0029b0, 0x0029b8, 0x0029b9, 
	0x0029c0, 0x0029c6, 0x0029c9, 0x0029ca, 0x0029ce, 0x0029d3, 0x0029d4, 0x0029d6, 0x0029d8, 0x0029dd, 0x0029e1, 0x0029e2, 0x0029e3, 0x0029e6, 0x0029e8, 0x0029ea, 
	0x0029f4, 0x0029fa, 0x0029fc, 0x0029fe, 0x002a0a, 0x002a1d, 0x002a1e, 0x002a22, 0x002a24, 0x002a25, 0x002a26, 0x002a27, 0x002a29, 0x002a2a, 0x002a2b, 0x002a2f, 
	0x002a34, 0x002a36, 0x002a3c, 0x002a3f, 0x002a57, 0x002a59, 0x002a64, 0x002a66, 0x002a6a, 0x002a6e, 0x002a6f, 0x002a71, 0x002a73, 0x002a75, 0x002a79, 0x002aa4, 
	0x002aa6, 0x002aae, 0x002aaf, 0x002ad7, 0x002adc, 0x002add, 0x002ade, 0x002adf, 0x002ae2, 0x002ae7, 0x002aec, 0x002aef, 0x002af3, 0x002af4, 0x002af7, 0x002afc, 
	0x002afd, 0x002afe, 0x002bfe, 0x002bff, 0x002e02, 0x002e06, 0x002e09, 0x002e0b, 0x002e0c, 0x002e0e, 0x002e1c, 0x002e1e, 0x002e20, 0x002e2a, 0x002e55, 0x002e5d, 
	0x003008, 0x003012, 0x003014, 0x00301c, 0x00fe59, 0x00fe5f, 0x00fe64, 0x00fe66, 0x00ff08, 0x00ff0a, 0x00ff1c, 0x00ff1d, 0x00ff1e, 0x00ff1f, 0x00ff3b, 0x00ff3c, 
	0x00ff3d, 0x00ff3e, 0x00ff5b, 0x00ff5c, 0x00ff5d, 0x00ff5e, 0x00ff5f, 0x00ff61, 0x00ff62, 0x00ff64, 0x01d6db, 0x01d6dc, 0x01d715, 0x01d716, 0x01d74f, 0x01d750, 
	0x01d789, 0x01d78a, 0x01d7c3, 0x01d7c4, 
};
#endif
#define mxCharSet_Binary_Property_Case_Ignorable 874
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Case_Ignorable[mxCharSet_Binary_Property_Case_Ignorable] = {
	0x000027, 0x000028, 0x00002e, 0x00002f, 0x00003a, 0x00003b, 0x00005e, 0x00005f, 0x000060, 0x000061, 0x0000a8, 0x0000a9, 0x0000ad, 0x0000ae, 0x0000af, 0x0000b0, 
	0x0000b4, 0x0000b5, 0x0000b7, 0x0000b9, 0x0002b0, 0x000370, 0x000374, 0x000376, 0x00037a, 0x00037b, 0x000384, 0x000386, 0x000387, 0x000388, 0x000483, 0x00048a, 
	0x000559, 0x00055a, 0x00055f, 0x000560, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x0005f4, 0x0005f5, 
	0x000600, 0x000606, 0x000610, 0x00061b, 0x00061c, 0x00061d, 0x000640, 0x000641, 0x00064b, 0x000660, 0x000670, 0x000671, 0x0006d6, 0x0006de, 0x0006df, 0x0006e9, 
	0x0006ea, 0x0006ee, 0x00070f, 0x000710, 0x000711, 0x000712, 0x000730, 0x00074b, 0x0007a6, 0x0007b1, 0x0007eb, 0x0007f6, 0x0007fa, 0x0007fb, 0x0007fd, 0x0007fe, 
	0x000816, 0x00082e, 0x000859, 0x00085c, 0x000888, 0x000889, 0x000890, 0x000892, 0x000898, 0x0008a0, 0x0008c9, 0x000903, 0x00093a, 0x00093b, 0x00093c, 0x00093d, 
	0x000941, 0x000949, 0x00094d, 0x00094e, 0x000951, 0x000958, 0x000962, 0x000964, 0x000971, 0x000972, 0x000981, 0x000982, 0x0009bc, 0x0009bd, 0x0009c1, 0x0009c5, 
	0x0009cd, 0x0009ce, 0x0009e2, 0x0009e4, 0x0009fe, 0x0009ff, 0x000a01, 0x000a03, 0x000a3c, 0x000a3d, 0x000a41, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 
	0x000a51, 0x000a52, 0x000a70, 0x000a72, 0x000a75, 0x000a76, 0x000a81, 0x000a83, 0x000abc, 0x000abd, 0x000ac1, 0x000ac6, 0x000ac7, 0x000ac9, 0x000acd, 0x000ace, 
	0x000ae2, 0x000ae4, 0x000afa, 0x000b00, 0x000b01, 0x000b02, 0x000b3c, 0x000b3d, 0x000b3f, 0x000b40, 0x000b41, 0x000b45, 0x000b4d, 0x000b4e, 0x000b55, 0x000b57, 
	0x000b62, 0x000b64, 0x000b82, 0x000b83, 0x000bc0, 0x000bc1, 0x000bcd, 0x000bce, 0x000c00, 0x000c01, 0x000c04, 0x000c05, 0x000c3c, 0x000c3d, 0x000c3e, 0x000c41, 
	0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c62, 0x000c64, 0x000c81, 0x000c82, 0x000cbc, 0x000cbd, 0x000cbf, 0x000cc0, 0x000cc6, 0x000cc7, 
	0x000ccc, 0x000cce, 0x000ce2, 0x000ce4, 0x000d00, 0x000d02, 0x000d3b, 0x000d3d, 0x000d41, 0x000d45, 0x000d4d, 0x000d4e, 0x000d62, 0x000d64, 0x000d81, 0x000d82, 
	0x000dca, 0x000dcb, 0x000dd2, 0x000dd5, 0x000dd6, 0x000dd7, 0x000e31, 0x000e32, 0x000e34, 0x000e3b, 0x000e46, 0x000e4f, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 
	0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 0x000f18, 0x000f1a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 0x000f71, 0x000f7f, 0x000f80, 0x000f85, 
	0x000f86, 0x000f88, 0x000f8d, 0x000f98, 0x000f99, 0x000fbd, 0x000fc6, 0x000fc7, 0x00102d, 0x001031, 0x001032, 0x001038, 0x001039, 0x00103b, 0x00103d, 0x00103f, 
	0x001058, 0x00105a, 0x00105e, 0x001061, 0x001071, 0x001075, 0x001082, 0x001083, 0x001085, 0x001087, 0x00108d, 0x00108e, 0x00109d, 0x00109e, 0x0010fc, 0x0010fd, 
	0x00135d, 0x001360, 0x001712, 0x001715, 0x001732, 0x001734, 0x001752, 0x001754, 0x001772, 0x001774, 0x0017b4, 0x0017b6, 0x0017b7, 0x0017be, 0x0017c6, 0x0017c7, 
	0x0017c9, 0x0017d4, 0x0017d7, 0x0017d8, 0x0017dd, 0x0017de, 0x00180b, 0x001810, 0x001843, 0x001844, 0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x001920, 0x001923, 
	0x001927, 0x001929, 0x001932, 0x001933, 0x001939, 0x00193c, 0x001a17, 0x001a19, 0x001a1b, 0x001a1c, 0x001a56, 0x001a57, 0x001a58, 0x001a5f, 0x001a60, 0x001a61, 
	0x001a62, 0x001a63, 0x001a65, 0x001a6d, 0x001a73, 0x001a7d, 0x001a7f, 0x001a80, 0x001aa7, 0x001aa8, 0x001ab0, 0x001acf, 0x001b00, 0x001b04, 0x001b34, 0x001b35, 
	0x001b36, 0x001b3b, 0x001b3c, 0x001b3d, 0x001b42, 0x001b43, 0x001b6b, 0x001b74, 0x001b80, 0x001b82, 0x001ba2, 0x001ba6, 0x001ba8, 0x001baa, 0x001bab, 0x001bae, 
	0x001be6, 0x001be7, 0x001be8, 0x001bea, 0x001bed, 0x001bee, 0x001bef, 0x001bf2, 0x001c2c, 0x001c34, 0x001c36, 0x001c38, 0x001c78, 0x001c7e, 0x001cd0, 0x001cd3, 
	0x001cd4, 0x001ce1, 0x001ce2, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001d2c, 0x001d6b, 0x001d78, 0x001d79, 0x001d9b, 0x001e00, 
	0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fcd, 0x001fd0, 0x001fdd, 0x001fe0, 0x001fed, 0x001ff0, 0x001ffd, 0x001fff, 0x00200b, 0x002010, 0x002018, 0x00201a, 
	0x002024, 0x002025, 0x002027, 0x002028, 0x00202a, 0x00202f, 0x002060, 0x002065, 0x002066, 0x002070, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 
	0x0020d0, 0x0020f1, 0x002c7c, 0x002c7e, 0x002cef, 0x002cf2, 0x002d6f, 0x002d70, 0x002d7f, 0x002d80, 0x002de0, 0x002e00, 0x002e2f, 0x002e30, 0x003005, 0x003006, 
	0x00302a, 0x00302e, 0x003031, 0x003036, 0x00303b, 0x00303c, 0x003099, 0x00309f, 0x0030fc, 0x0030ff, 0x00a015, 0x00a016, 0x00a4f8, 0x00a4fe, 0x00a60c, 0x00a60d, 
	0x00a66f, 0x00a673, 0x00a674, 0x00a67e, 0x00a67f, 0x00a680, 0x00a69c, 0x00a6a0, 0x00a6f0, 0x00a6f2, 0x00a700, 0x00a722, 0x00a770, 0x00a771, 0x00a788, 0x00a78b, 
	0x00a7f2, 0x00a7f5, 0x00a7f8, 0x00a7fa, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a825, 0x00a827, 0x00a82c, 0x00a82d, 0x00a8c4, 0x00a8c6, 
	0x00a8e0, 0x00a8f2, 0x00a8ff, 0x00a900, 0x00a926, 0x00a92e, 0x00a947, 0x00a952, 0x00a980, 0x00a983, 0x00a9b3, 0x00a9b4, 0x00a9b6, 0x00a9ba, 0x00a9bc, 0x00a9be, 
	0x00a9cf, 0x00a9d0, 0x00a9e5, 0x00a9e7, 0x00aa29, 0x00aa2f, 0x00aa31, 0x00aa33, 0x00aa35, 0x00aa37, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa4d, 0x00aa70, 0x00aa71, 
	0x00aa7c, 0x00aa7d, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aadd, 0x00aade, 0x00aaec, 0x00aaee, 
	0x00aaf3, 0x00aaf5, 0x00aaf6, 0x00aaf7, 0x00ab5b, 0x00ab60, 0x00ab69, 0x00ab6c, 0x00abe5, 0x00abe6, 0x00abe8, 0x00abe9, 0x00abed, 0x00abee, 0x00fb1e, 0x00fb1f, 
	0x00fbb2, 0x00fbc3, 0x00fe00, 0x00fe10, 0x00fe13, 0x00fe14, 0x00fe20, 0x00fe30, 0x00fe52, 0x00fe53, 0x00fe55, 0x00fe56, 0x00feff, 0x00ff00, 0x00ff07, 0x00ff08, 
	0x00ff0e, 0x00ff0f, 0x00ff1a, 0x00ff1b, 0x00ff3e, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ff70, 0x00ff71, 0x00ff9e, 0x00ffa0, 0x00ffe3, 0x00ffe4, 0x00fff9, 0x00fffc, 
	0x0101fd, 0x0101fe, 0x0102e0, 0x0102e1, 0x010376, 0x01037b, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010a01, 0x010a04, 0x010a05, 0x010a07, 
	0x010a0c, 0x010a10, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 0x010ae5, 0x010ae7, 0x010d24, 0x010d28, 0x010eab, 0x010ead, 0x010efd, 0x010f00, 0x010f46, 0x010f51, 
	0x010f82, 0x010f86, 0x011001, 0x011002, 0x011038, 0x011047, 0x011070, 0x011071, 0x011073, 0x011075, 0x01107f, 0x011082, 0x0110b3, 0x0110b7, 0x0110b9, 0x0110bb, 
	0x0110bd, 0x0110be, 0x0110c2, 0x0110c3, 0x0110cd, 0x0110ce, 0x011100, 0x011103, 0x011127, 0x01112c, 0x01112d, 0x011135, 0x011173, 0x011174, 0x011180, 0x011182, 
	0x0111b6, 0x0111bf, 0x0111c9, 0x0111cd, 0x0111cf, 0x0111d0, 0x01122f, 0x011232, 0x011234, 0x011235, 0x011236, 0x011238, 0x01123e, 0x01123f, 0x011241, 0x011242, 
	0x0112df, 0x0112e0, 0x0112e3, 0x0112eb, 0x011300, 0x011302, 0x01133b, 0x01133d, 0x011340, 0x011341, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011438, 0x011440, 
	0x011442, 0x011445, 0x011446, 0x011447, 0x01145e, 0x01145f, 0x0114b3, 0x0114b9, 0x0114ba, 0x0114bb, 0x0114bf, 0x0114c1, 0x0114c2, 0x0114c4, 0x0115b2, 0x0115b6, 
	0x0115bc, 0x0115be, 0x0115bf, 0x0115c1, 0x0115dc, 0x0115de, 0x011633, 0x01163b, 0x01163d, 0x01163e, 0x01163f, 0x011641, 0x0116ab, 0x0116ac, 0x0116ad, 0x0116ae, 
	0x0116b0, 0x0116b6, 0x0116b7, 0x0116b8, 0x01171d, 0x011720, 0x011722, 0x011726, 0x011727, 0x01172c, 0x01182f, 0x011838, 0x011839, 0x01183b, 0x01193b, 0x01193d, 
	0x01193e, 0x01193f, 0x011943, 0x011944, 0x0119d4, 0x0119d8, 0x0119da, 0x0119dc, 0x0119e0, 0x0119e1, 0x011a01, 0x011a0b, 0x011a33, 0x011a39, 0x011a3b, 0x011a3f, 
	0x011a47, 0x011a48, 0x011a51, 0x011a57, 0x011a59, 0x011a5c, 0x011a8a, 0x011a97, 0x011a98, 0x011a9a, 0x011c30, 0x011c37, 0x011c38, 0x011c3e, 0x011c3f, 0x011c40, 
	0x011c92, 0x011ca8, 0x011caa, 0x011cb1, 0x011cb2, 0x011cb4, 0x011cb5, 0x011cb7, 0x011d31, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d46, 
	0x011d47, 0x011d48, 0x011d90, 0x011d92, 0x011d95, 0x011d96, 0x011d97, 0x011d98, 0x011ef3, 0x011ef5, 0x011f00, 0x011f02, 0x011f36, 0x011f3b, 0x011f40, 0x011f41, 
	0x011f42, 0x011f43, 0x013430, 0x013441, 0x013447, 0x013456, 0x016af0, 0x016af5, 0x016b30, 0x016b37, 0x016b40, 0x016b44, 0x016f4f, 0x016f50, 0x016f8f, 0x016fa0, 
	0x016fe0, 0x016fe2, 0x016fe3, 0x016fe5, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01bc9d, 0x01bc9f, 0x01bca0, 0x01bca4, 0x01cf00, 0x01cf2e, 
	0x01cf30, 0x01cf47, 0x01d167, 0x01d16a, 0x01d173, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 0x01da00, 0x01da37, 0x01da3b, 0x01da6d, 
	0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 
	0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e130, 0x01e13e, 0x01e2ae, 0x01e2af, 0x01e2ec, 0x01e2f0, 0x01e4eb, 0x01e4f0, 0x01e8d0, 0x01e8d7, 
	0x01e944, 0x01e94c, 0x01f3fb, 0x01f400, 0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Binary_Property_Cased 314
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Cased[mxCharSet_Binary_Property_Cased] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0001bb, 
	0x0001bc, 0x0001c0, 0x0001c4, 0x000294, 0x000295, 0x0002b9, 0x0002c0, 0x0002c2, 0x0002e0, 0x0002e5, 0x000345, 0x000346, 0x000370, 0x000374, 0x000376, 0x000378, 
	0x00037a, 0x00037e, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 
	0x00048a, 0x000530, 0x000531, 0x000557, 0x000560, 0x000589, 0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001100, 
	0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001d00, 0x001dc0, 0x001e00, 0x001f16, 0x001f18, 0x001f1e, 
	0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 
	0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 
	0x001ff6, 0x001ffd, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 
	0x002119, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x002135, 0x002139, 0x00213a, 0x00213c, 0x002140, 
	0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002160, 0x002180, 0x002183, 0x002185, 0x0024b6, 0x0024ea, 0x002c00, 0x002ce5, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 
	0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x00a640, 0x00a66e, 0x00a680, 0x00a69e, 0x00a722, 0x00a788, 0x00a78b, 0x00a78f, 0x00a790, 0x00a7cb, 
	0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a7f7, 0x00a7f8, 0x00a7fb, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abc0, 
	0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x010400, 0x010450, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010570, 0x01057b, 
	0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010780, 0x010781, 
	0x010783, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x0118a0, 0x0118e0, 0x016e40, 0x016e80, 0x01d400, 0x01d455, 
	0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 
	0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 
	0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 
	0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01df00, 0x01df0a, 0x01df0b, 0x01df1f, 0x01df25, 0x01df2b, 
	0x01e030, 0x01e06e, 0x01e900, 0x01e944, 0x01f130, 0x01f14a, 0x01f150, 0x01f16a, 0x01f170, 0x01f18a, 
};
#ifdef mxRegExpUnicodePropertyEscapes
#define mxCharSet_Binary_Property_Changes_When_Casefolded 1244
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_Casefolded[mxCharSet_Binary_Property_Changes_When_Casefolded] = {
	0x000041, 0x00005b, 0x0000b5, 0x0000b6, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000e0, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 
	0x000108, 0x000109, 0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 
	0x000118, 0x000119, 0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 
	0x000128, 0x000129, 0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 
	0x000139, 0x00013a, 0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 
	0x000149, 0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 
	0x00015a, 0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 
	0x00016a, 0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 
	0x00017b, 0x00017c, 0x00017d, 0x00017e, 0x00017f, 0x000180, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 
	0x000193, 0x000195, 0x000196, 0x000199, 0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 
	0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bc, 0x0001bd, 0x0001c4, 0x0001c6, 0x0001c7, 0x0001c9, 
	0x0001ca, 0x0001cc, 0x0001cd, 0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 
	0x0001db, 0x0001dc, 0x0001de, 0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 
	0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 0x0001f1, 0x0001f3, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 
	0x000200, 0x000201, 0x000202, 0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 
	0x000210, 0x000211, 0x000212, 0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 
	0x000220, 0x000221, 0x000222, 0x000223, 0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 
	0x000230, 0x000231, 0x000232, 0x000233, 0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 
	0x00024c, 0x00024d, 0x00024e, 0x00024f, 0x000345, 0x000346, 0x000370, 0x000371, 0x000372, 0x000373, 0x000376, 0x000377, 0x00037f, 0x000380, 0x000386, 0x000387, 
	0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x000390, 0x000391, 0x0003a2, 0x0003a3, 0x0003ac, 0x0003c2, 0x0003c3, 0x0003cf, 0x0003d2, 0x0003d5, 0x0003d7, 
	0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 
	0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f0, 0x0003f2, 0x0003f4, 0x0003f6, 0x0003f7, 0x0003f8, 0x0003f9, 0x0003fb, 
	0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 
	0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 
	0x00047e, 0x00047f, 0x000480, 0x000481, 0x00048a, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 
	0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 
	0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 
	0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 
	0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 
	0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 
	0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 
	0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 
	0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 
	0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 
	0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000531, 0x000557, 0x000587, 0x000588, 0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 
	0x0010cd, 0x0010ce, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001e00, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 
	0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 
	0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 
	0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 
	0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 
	0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 
	0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 
	0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 
	0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 
	0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 
	0x001e9a, 0x001e9c, 0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 
	0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 
	0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 
	0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 
	0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 
	0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 
	0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f18, 0x001f1e, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f48, 0x001f4e, 0x001f59, 0x001f5a, 
	0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f60, 0x001f68, 0x001f70, 0x001f80, 0x001fb0, 0x001fb2, 0x001fb5, 0x001fb7, 0x001fbd, 0x001fc2, 0x001fc5, 
	0x001fc7, 0x001fcd, 0x001fd8, 0x001fdc, 0x001fe8, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff7, 0x001ffd, 0x002126, 0x002127, 0x00212a, 0x00212c, 0x002132, 0x002133, 
	0x002160, 0x002170, 0x002183, 0x002184, 0x0024b6, 0x0024d0, 0x002c00, 0x002c30, 0x002c60, 0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 
	0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7e, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 
	0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 
	0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 
	0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 
	0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 
	0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 
	0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ceb, 0x002cec, 0x002ced, 0x002cee, 
	0x002cf2, 0x002cf3, 0x00a640, 0x00a641, 0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 
	0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 
	0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 
	0x00a680, 0x00a681, 0x00a682, 0x00a683, 0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 
	0x00a690, 0x00a691, 0x00a692, 0x00a693, 0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a722, 0x00a723, 0x00a724, 0x00a725, 
	0x00a726, 0x00a727, 0x00a728, 0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 
	0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 
	0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 
	0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 
	0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 
	0x00a782, 0x00a783, 0x00a784, 0x00a785, 0x00a786, 0x00a787, 0x00a78b, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a790, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 
	0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 
	0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7af, 0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 
	0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7d0, 0x00a7d1, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7f5, 0x00a7f6, 
	0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00ff21, 0x00ff3b, 0x010400, 0x010428, 0x0104b0, 0x0104d4, 0x010570, 0x01057b, 0x01057c, 0x01058b, 
	0x01058c, 0x010593, 0x010594, 0x010596, 0x010c80, 0x010cb3, 0x0118a0, 0x0118c0, 0x016e40, 0x016e60, 0x01e900, 0x01e922, 
};
#define mxCharSet_Binary_Property_Changes_When_Casemapped 262
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_Casemapped[mxCharSet_Binary_Property_Changes_When_Casemapped] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000b5, 0x0000b6, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x000138, 0x000139, 0x00018d, 0x00018e, 0x00019b, 
	0x00019c, 0x0001aa, 0x0001ac, 0x0001ba, 0x0001bc, 0x0001be, 0x0001bf, 0x0001c0, 0x0001c4, 0x000221, 0x000222, 0x000234, 0x00023a, 0x000255, 0x000256, 0x000258, 
	0x000259, 0x00025a, 0x00025b, 0x00025d, 0x000260, 0x000262, 0x000263, 0x000264, 0x000265, 0x000267, 0x000268, 0x00026d, 0x00026f, 0x000270, 0x000271, 0x000273, 
	0x000275, 0x000276, 0x00027d, 0x00027e, 0x000280, 0x000281, 0x000282, 0x000284, 0x000287, 0x00028d, 0x000292, 0x000293, 0x00029d, 0x00029f, 0x000345, 0x000346, 
	0x000370, 0x000374, 0x000376, 0x000378, 0x00037b, 0x00037e, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 
	0x0003a3, 0x0003d2, 0x0003d5, 0x0003f6, 0x0003f7, 0x0003fc, 0x0003fd, 0x000482, 0x00048a, 0x000530, 0x000531, 0x000557, 0x000561, 0x000588, 0x0010a0, 0x0010c6, 
	0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fd, 0x001100, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 
	0x001cbd, 0x001cc0, 0x001d79, 0x001d7a, 0x001d7d, 0x001d7e, 0x001d8e, 0x001d8f, 0x001e00, 0x001e9c, 0x001e9e, 0x001e9f, 0x001ea0, 0x001f16, 0x001f18, 0x001f1e, 
	0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 
	0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 
	0x001ff6, 0x001ffd, 0x002126, 0x002127, 0x00212a, 0x00212c, 0x002132, 0x002133, 0x00214e, 0x00214f, 0x002160, 0x002180, 0x002183, 0x002185, 0x0024b6, 0x0024ea, 
	0x002c00, 0x002c71, 0x002c72, 0x002c74, 0x002c75, 0x002c77, 0x002c7e, 0x002ce4, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 
	0x002d2d, 0x002d2e, 0x00a640, 0x00a66e, 0x00a680, 0x00a69c, 0x00a722, 0x00a730, 0x00a732, 0x00a770, 0x00a779, 0x00a788, 0x00a78b, 0x00a78e, 0x00a790, 0x00a795, 
	0x00a796, 0x00a7af, 0x00a7b0, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d6, 0x00a7da, 0x00a7f5, 0x00a7f7, 0x00ab53, 0x00ab54, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 
	0x00fb13, 0x00fb18, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x010400, 0x010450, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010570, 0x01057b, 0x01057c, 0x01058b, 
	0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 
	0x0118a0, 0x0118e0, 0x016e40, 0x016e80, 0x01e900, 0x01e944, 
};
#define mxCharSet_Binary_Property_Changes_When_Lowercased 1218
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_Lowercased[mxCharSet_Binary_Property_Changes_When_Lowercased] = {
	0x000041, 0x00005b, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000df, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 0x000109, 
	0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 0x000119, 
	0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 0x000129, 
	0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000139, 0x00013a, 
	0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 0x00014b, 
	0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 0x00015b, 
	0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 0x00016b, 
	0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 0x00017c, 
	0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 0x000193, 0x000195, 0x000196, 0x000199, 
	0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 
	0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bc, 0x0001bd, 0x0001c4, 0x0001c6, 0x0001c7, 0x0001c9, 0x0001ca, 0x0001cc, 0x0001cd, 0x0001ce, 
	0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 0x0001df, 
	0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 
	0x0001f1, 0x0001f3, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 0x000203, 
	0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 0x000213, 
	0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 0x000221, 0x000222, 0x000223, 
	0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 
	0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 
	0x000370, 0x000371, 0x000372, 0x000373, 0x000376, 0x000377, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x000390, 
	0x000391, 0x0003a2, 0x0003a3, 0x0003ac, 0x0003cf, 0x0003d0, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 
	0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f4, 0x0003f5, 
	0x0003f7, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 
	0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 
	0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x00048a, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 
	0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 
	0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 
	0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 
	0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 
	0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 
	0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 
	0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 
	0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 
	0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 
	0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000531, 0x000557, 0x0010a0, 0x0010c6, 
	0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0013a0, 0x0013f6, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001e00, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 
	0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 
	0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 
	0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 
	0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 
	0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 
	0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 
	0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 
	0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 
	0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 
	0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 
	0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 
	0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 
	0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 
	0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 
	0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 
	0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f18, 0x001f1e, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f48, 0x001f4e, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 
	0x001f5d, 0x001f5e, 0x001f5f, 0x001f60, 0x001f68, 0x001f70, 0x001f88, 0x001f90, 0x001f98, 0x001fa0, 0x001fa8, 0x001fb0, 0x001fb8, 0x001fbd, 0x001fc8, 0x001fcd, 
	0x001fd8, 0x001fdc, 0x001fe8, 0x001fed, 0x001ff8, 0x001ffd, 0x002126, 0x002127, 0x00212a, 0x00212c, 0x002132, 0x002133, 0x002160, 0x002170, 0x002183, 0x002184, 
	0x0024b6, 0x0024d0, 0x002c00, 0x002c30, 0x002c60, 0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 
	0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7e, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 
	0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 
	0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 
	0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 
	0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 
	0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 
	0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ceb, 0x002cec, 0x002ced, 0x002cee, 0x002cf2, 0x002cf3, 0x00a640, 0x00a641, 
	0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 
	0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 
	0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a680, 0x00a681, 0x00a682, 0x00a683, 
	0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 
	0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a722, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 0x00a729, 
	0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 
	0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 
	0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 
	0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 
	0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 0x00a785, 
	0x00a786, 0x00a787, 0x00a78b, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a790, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 
	0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7af, 
	0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 
	0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7d0, 0x00a7d1, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7f5, 0x00a7f6, 0x00ff21, 0x00ff3b, 0x010400, 0x010428, 
	0x0104b0, 0x0104d4, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010c80, 0x010cb3, 0x0118a0, 0x0118c0, 0x016e40, 0x016e60, 
	0x01e900, 0x01e922, 
};
#define mxCharSet_Binary_Property_Changes_When_NFKC_Casefolded 1678
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_NFKC_Casefolded[mxCharSet_Binary_Property_Changes_When_NFKC_Casefolded] = {
	0x000041, 0x00005b, 0x0000a0, 0x0000a1, 0x0000a8, 0x0000a9, 0x0000aa, 0x0000ab, 0x0000ad, 0x0000ae, 0x0000af, 0x0000b0, 0x0000b2, 0x0000b6, 0x0000b8, 0x0000bb, 
	0x0000bc, 0x0000bf, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000e0, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 0x000109, 
	0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 0x000119, 
	0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 0x000129, 
	0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000135, 0x000136, 0x000137, 0x000139, 0x00013a, 0x00013b, 0x00013c, 
	0x00013d, 0x00013e, 0x00013f, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x000149, 0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 
	0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 
	0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 
	0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 0x00017c, 0x00017d, 0x00017e, 0x00017f, 0x000180, 
	0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 0x000193, 0x000195, 0x000196, 0x000199, 0x00019c, 0x00019e, 
	0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 0x0001b1, 0x0001b4, 
	0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bc, 0x0001bd, 0x0001c4, 0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 
	0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 
	0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 0x0001f1, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 
	0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 
	0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 
	0x00021e, 0x00021f, 0x000220, 0x000221, 0x000222, 0x000223, 0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 
	0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 
	0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 0x0002b0, 0x0002b9, 0x0002d8, 0x0002de, 0x0002e0, 0x0002e5, 0x000340, 0x000342, 0x000343, 0x000346, 
	0x00034f, 0x000350, 0x000370, 0x000371, 0x000372, 0x000373, 0x000374, 0x000375, 0x000376, 0x000377, 0x00037a, 0x00037b, 0x00037e, 0x000380, 0x000384, 0x00038b, 
	0x00038c, 0x00038d, 0x00038e, 0x000390, 0x000391, 0x0003a2, 0x0003a3, 0x0003ac, 0x0003c2, 0x0003c3, 0x0003cf, 0x0003d7, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 
	0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 
	0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f0, 0x0003f3, 0x0003f4, 0x0003f6, 0x0003f7, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 
	0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 
	0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 
	0x00048a, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 
	0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 
	0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 
	0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 
	0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 
	0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 
	0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 
	0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 
	0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 
	0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 
	0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000531, 0x000557, 0x000587, 0x000588, 0x00061c, 0x00061d, 0x000675, 0x000679, 0x000958, 0x000960, 0x0009dc, 0x0009de, 
	0x0009df, 0x0009e0, 0x000a33, 0x000a34, 0x000a36, 0x000a37, 0x000a59, 0x000a5c, 0x000a5e, 0x000a5f, 0x000b5c, 0x000b5e, 0x000e33, 0x000e34, 0x000eb3, 0x000eb4, 
	0x000edc, 0x000ede, 0x000f0c, 0x000f0d, 0x000f43, 0x000f44, 0x000f4d, 0x000f4e, 0x000f52, 0x000f53, 0x000f57, 0x000f58, 0x000f5c, 0x000f5d, 0x000f69, 0x000f6a, 
	0x000f73, 0x000f74, 0x000f75, 0x000f7a, 0x000f81, 0x000f82, 0x000f93, 0x000f94, 0x000f9d, 0x000f9e, 0x000fa2, 0x000fa3, 0x000fa7, 0x000fa8, 0x000fac, 0x000fad, 
	0x000fb9, 0x000fba, 0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010fc, 0x0010fd, 0x00115f, 0x001161, 0x0013f8, 0x0013fe, 0x0017b4, 0x0017b6, 
	0x00180b, 0x001810, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001d2c, 0x001d2f, 0x001d30, 0x001d3b, 0x001d3c, 0x001d4e, 0x001d4f, 0x001d6b, 
	0x001d78, 0x001d79, 0x001d9b, 0x001dc0, 0x001e00, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 
	0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 
	0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 
	0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 
	0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 
	0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 
	0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 
	0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 
	0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 
	0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 0x001e9a, 0x001e9c, 0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 
	0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 
	0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 
	0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 
	0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 
	0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 
	0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 
	0x001f18, 0x001f1e, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f48, 0x001f4e, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f60, 
	0x001f68, 0x001f70, 0x001f71, 0x001f72, 0x001f73, 0x001f74, 0x001f75, 0x001f76, 0x001f77, 0x001f78, 0x001f79, 0x001f7a, 0x001f7b, 0x001f7c, 0x001f7d, 0x001f7e, 
	0x001f80, 0x001fb0, 0x001fb2, 0x001fb5, 0x001fb7, 0x001fc5, 0x001fc7, 0x001fd0, 0x001fd3, 0x001fd4, 0x001fd8, 0x001fdc, 0x001fdd, 0x001fe0, 0x001fe3, 0x001fe4, 
	0x001fe8, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff7, 0x001fff, 0x002000, 0x002010, 0x002011, 0x002012, 0x002017, 0x002018, 0x002024, 0x002027, 0x00202a, 0x002030, 
	0x002033, 0x002035, 0x002036, 0x002038, 0x00203c, 0x00203d, 0x00203e, 0x00203f, 0x002047, 0x00204a, 0x002057, 0x002058, 0x00205f, 0x002072, 0x002074, 0x00208f, 
	0x002090, 0x00209d, 0x0020a8, 0x0020a9, 0x002100, 0x002104, 0x002105, 0x002108, 0x002109, 0x002114, 0x002115, 0x002117, 0x002119, 0x00211e, 0x002120, 0x002123, 
	0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x00213a, 0x00213b, 0x002141, 0x002145, 0x00214a, 0x002150, 0x002180, 
	0x002183, 0x002184, 0x002189, 0x00218a, 0x00222c, 0x00222e, 0x00222f, 0x002231, 0x002329, 0x00232b, 0x002460, 0x0024eb, 0x002a0c, 0x002a0d, 0x002a74, 0x002a77, 
	0x002adc, 0x002add, 0x002c00, 0x002c30, 0x002c60, 0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 
	0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7c, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 
	0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 
	0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 
	0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 
	0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 
	0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 
	0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ceb, 0x002cec, 0x002ced, 0x002cee, 0x002cf2, 0x002cf3, 0x002d6f, 0x002d70, 
	0x002e9f, 0x002ea0, 0x002ef3, 0x002ef4, 0x002f00, 0x002fd6, 0x003000, 0x003001, 0x003036, 0x003037, 0x003038, 0x00303b, 0x00309b, 0x00309d, 0x00309f, 0x0030a0, 
	0x0030ff, 0x003100, 0x003131, 0x00318f, 0x003192, 0x0031a0, 0x003200, 0x00321f, 0x003220, 0x003248, 0x003250, 0x00327f, 0x003280, 0x003400, 0x00a640, 0x00a641, 
	0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 
	0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 
	0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a680, 0x00a681, 0x00a682, 0x00a683, 
	0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 
	0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a69c, 0x00a69e, 0x00a722, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 
	0x00a728, 0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 
	0x00a73a, 0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 
	0x00a74a, 0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 
	0x00a75a, 0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 
	0x00a76a, 0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a770, 0x00a771, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 
	0x00a782, 0x00a783, 0x00a784, 0x00a785, 0x00a786, 0x00a787, 0x00a78b, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a790, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 
	0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 
	0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7af, 0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 
	0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7d0, 0x00a7d1, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7f2, 0x00a7f6, 
	0x00a7f8, 0x00a7fa, 0x00ab5c, 0x00ab60, 0x00ab69, 0x00ab6a, 0x00ab70, 0x00abc0, 0x00f900, 0x00fa0e, 0x00fa10, 0x00fa11, 0x00fa12, 0x00fa13, 0x00fa15, 0x00fa1f, 
	0x00fa20, 0x00fa21, 0x00fa22, 0x00fa23, 0x00fa25, 0x00fa27, 0x00fa2a, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb1e, 
	0x00fb1f, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 0x00fd50, 0x00fd90, 
	0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfd, 0x00fe00, 0x00fe1a, 0x00fe30, 0x00fe45, 0x00fe47, 0x00fe53, 0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 0x00fe70, 0x00fe73, 
	0x00fe74, 0x00fe75, 0x00fe76, 0x00fefd, 0x00feff, 0x00ff00, 0x00ff01, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 
	0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fff0, 0x00fff9, 0x010400, 0x010428, 0x0104b0, 0x0104d4, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 
	0x010594, 0x010596, 0x010781, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010c80, 0x010cb3, 0x0118a0, 0x0118c0, 0x016e40, 0x016e60, 0x01bca0, 0x01bca4, 
	0x01d15e, 0x01d165, 0x01d173, 0x01d17b, 0x01d1bb, 0x01d1c1, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 
	0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 
	0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01d800, 
	0x01e030, 0x01e06e, 0x01e900, 0x01e922, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 
	0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 
	0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 
	0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 
	0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01f100, 0x01f10b, 0x01f110, 0x01f12f, 0x01f130, 0x01f150, 0x01f16a, 0x01f16d, 0x01f190, 0x01f191, 
	0x01f200, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01fbf0, 0x01fbfa, 0x02f800, 0x02fa1e, 0x0e0000, 0x0e1000, 
};
#define mxCharSet_Binary_Property_Changes_When_Titlecased 1252
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_Titlecased[mxCharSet_Binary_Property_Changes_When_Titlecased] = {
	0x000061, 0x00007b, 0x0000b5, 0x0000b6, 0x0000df, 0x0000f7, 0x0000f8, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 
	0x000109, 0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 
	0x000119, 0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 
	0x000129, 0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000138, 
	0x00013a, 0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 
	0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 
	0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 
	0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 
	0x00017c, 0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018d, 0x000192, 0x000193, 0x000195, 0x000196, 
	0x000199, 0x00019b, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001ad, 0x0001ae, 0x0001b0, 0x0001b1, 
	0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001ba, 0x0001bd, 0x0001be, 0x0001bf, 0x0001c0, 0x0001c4, 0x0001c5, 0x0001c6, 0x0001c8, 0x0001c9, 0x0001cb, 
	0x0001cc, 0x0001cd, 0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 
	0x0001dc, 0x0001de, 0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 
	0x0001ed, 0x0001ee, 0x0001ef, 0x0001f2, 0x0001f3, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 
	0x000201, 0x000202, 0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 
	0x000211, 0x000212, 0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 
	0x000223, 0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 
	0x000233, 0x000234, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 
	0x00024f, 0x000255, 0x000256, 0x000258, 0x000259, 0x00025a, 0x00025b, 0x00025d, 0x000260, 0x000262, 0x000263, 0x000264, 0x000265, 0x000267, 0x000268, 0x00026d, 
	0x00026f, 0x000270, 0x000271, 0x000273, 0x000275, 0x000276, 0x00027d, 0x00027e, 0x000280, 0x000281, 0x000282, 0x000284, 0x000287, 0x00028d, 0x000292, 0x000293, 
	0x00029d, 0x00029f, 0x000345, 0x000346, 0x000371, 0x000372, 0x000373, 0x000374, 0x000377, 0x000378, 0x00037b, 0x00037e, 0x000390, 0x000391, 0x0003ac, 0x0003cf, 
	0x0003d0, 0x0003d2, 0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 
	0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f4, 0x0003f5, 0x0003f6, 0x0003f8, 0x0003f9, 
	0x0003fb, 0x0003fc, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 
	0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 
	0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x000482, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 
	0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 
	0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 
	0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 
	0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 
	0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 
	0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 
	0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 
	0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 
	0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 
	0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000530, 0x000561, 0x000588, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 
	0x001d79, 0x001d7a, 0x001d7d, 0x001d7e, 0x001d8e, 0x001d8f, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 
	0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 
	0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 
	0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 
	0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 
	0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 
	0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 
	0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 
	0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 
	0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 0x001e9c, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 
	0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 
	0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 
	0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 
	0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 
	0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 
	0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f16, 0x001f20, 0x001f28, 
	0x001f30, 0x001f38, 0x001f40, 0x001f46, 0x001f50, 0x001f58, 0x001f60, 0x001f68, 0x001f70, 0x001f7e, 0x001f80, 0x001f88, 0x001f90, 0x001f98, 0x001fa0, 0x001fa8, 
	0x001fb0, 0x001fb5, 0x001fb6, 0x001fb8, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fc8, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fd8, 0x001fe0, 0x001fe8, 
	0x001ff2, 0x001ff5, 0x001ff6, 0x001ff8, 0x00214e, 0x00214f, 0x002170, 0x002180, 0x002184, 0x002185, 0x0024d0, 0x0024ea, 0x002c30, 0x002c60, 0x002c61, 0x002c62, 
	0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c73, 0x002c74, 0x002c76, 0x002c77, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 
	0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 
	0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 
	0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 
	0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 
	0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 
	0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ce4, 
	0x002cec, 0x002ced, 0x002cee, 0x002cef, 0x002cf3, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x00a641, 0x00a642, 0x00a643, 0x00a644, 
	0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 0x00a652, 0x00a653, 0x00a654, 
	0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 0x00a662, 0x00a663, 0x00a664, 
	0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a66e, 0x00a681, 0x00a682, 0x00a683, 0x00a684, 0x00a685, 0x00a686, 
	0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 0x00a694, 0x00a695, 0x00a696, 
	0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a69c, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 
	0x00a72d, 0x00a72e, 0x00a72f, 0x00a730, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 
	0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 
	0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 
	0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 
	0x00a76f, 0x00a770, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 0x00a785, 0x00a786, 0x00a787, 0x00a788, 
	0x00a78c, 0x00a78d, 0x00a791, 0x00a792, 0x00a793, 0x00a795, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 
	0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 
	0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7cb, 0x00a7d1, 0x00a7d2, 
	0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7da, 0x00a7f6, 0x00a7f7, 0x00ab53, 0x00ab54, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00ff41, 0x00ff5b, 
	0x010428, 0x010450, 0x0104d8, 0x0104fc, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010cc0, 0x010cf3, 0x0118c0, 0x0118e0, 
	0x016e60, 0x016e80, 0x01e922, 0x01e944, 
};
#define mxCharSet_Binary_Property_Changes_When_Uppercased 1254
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Changes_When_Uppercased[mxCharSet_Binary_Property_Changes_When_Uppercased] = {
	0x000061, 0x00007b, 0x0000b5, 0x0000b6, 0x0000df, 0x0000f7, 0x0000f8, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 
	0x000109, 0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 
	0x000119, 0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 
	0x000129, 0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000138, 
	0x00013a, 0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 
	0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 
	0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 
	0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 
	0x00017c, 0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018d, 0x000192, 0x000193, 0x000195, 0x000196, 
	0x000199, 0x00019b, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001ad, 0x0001ae, 0x0001b0, 0x0001b1, 
	0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001ba, 0x0001bd, 0x0001be, 0x0001bf, 0x0001c0, 0x0001c5, 0x0001c7, 0x0001c8, 0x0001ca, 0x0001cb, 0x0001cd, 
	0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 
	0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 
	0x0001ef, 0x0001f1, 0x0001f2, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 
	0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 
	0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 0x000223, 0x000224, 
	0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 0x000234, 
	0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 0x000255, 
	0x000256, 0x000258, 0x000259, 0x00025a, 0x00025b, 0x00025d, 0x000260, 0x000262, 0x000263, 0x000264, 0x000265, 0x000267, 0x000268, 0x00026d, 0x00026f, 0x000270, 
	0x000271, 0x000273, 0x000275, 0x000276, 0x00027d, 0x00027e, 0x000280, 0x000281, 0x000282, 0x000284, 0x000287, 0x00028d, 0x000292, 0x000293, 0x00029d, 0x00029f, 
	0x000345, 0x000346, 0x000371, 0x000372, 0x000373, 0x000374, 0x000377, 0x000378, 0x00037b, 0x00037e, 0x000390, 0x000391, 0x0003ac, 0x0003cf, 0x0003d0, 0x0003d2, 
	0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 
	0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f4, 0x0003f5, 0x0003f6, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fc, 
	0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 
	0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 
	0x00047f, 0x000480, 0x000481, 0x000482, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 
	0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 
	0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 
	0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 
	0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 
	0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 
	0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 
	0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 
	0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 
	0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 
	0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000530, 0x000561, 0x000588, 0x0010d0, 0x0010fb, 0x0010fd, 0x001100, 0x0013f8, 0x0013fe, 
	0x001c80, 0x001c89, 0x001d79, 0x001d7a, 0x001d7d, 0x001d7e, 0x001d8e, 0x001d8f, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 
	0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 
	0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 
	0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 
	0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 
	0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 
	0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 
	0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 
	0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 
	0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 0x001e9c, 0x001ea1, 0x001ea2, 
	0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 
	0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 
	0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 
	0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 
	0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 
	0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f16, 
	0x001f20, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f46, 0x001f50, 0x001f58, 0x001f60, 0x001f68, 0x001f70, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fb8, 
	0x001fbc, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fc8, 0x001fcc, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fd8, 0x001fe0, 0x001fe8, 
	0x001ff2, 0x001ff5, 0x001ff6, 0x001ff8, 0x001ffc, 0x001ffd, 0x00214e, 0x00214f, 0x002170, 0x002180, 0x002184, 0x002185, 0x0024d0, 0x0024ea, 0x002c30, 0x002c60, 
	0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c73, 0x002c74, 0x002c76, 0x002c77, 0x002c81, 0x002c82, 
	0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 
	0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 
	0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 
	0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 
	0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 
	0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 
	0x002ce3, 0x002ce4, 0x002cec, 0x002ced, 0x002cee, 0x002cef, 0x002cf3, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x00a641, 0x00a642, 
	0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 0x00a652, 
	0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 0x00a662, 
	0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a66e, 0x00a681, 0x00a682, 0x00a683, 0x00a684, 
	0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 0x00a694, 
	0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a69c, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 0x00a729, 0x00a72a, 
	0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a730, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 0x00a73c, 
	0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 0x00a74c, 
	0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 0x00a75c, 
	0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 0x00a76c, 
	0x00a76d, 0x00a76e, 0x00a76f, 0x00a770, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 0x00a785, 0x00a786, 
	0x00a787, 0x00a788, 0x00a78c, 0x00a78d, 0x00a791, 0x00a792, 0x00a793, 0x00a795, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 
	0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 
	0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7cb, 
	0x00a7d1, 0x00a7d2, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7da, 0x00a7f6, 0x00a7f7, 0x00ab53, 0x00ab54, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 
	0x00ff41, 0x00ff5b, 0x010428, 0x010450, 0x0104d8, 0x0104fc, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010cc0, 0x010cf3, 
	0x0118c0, 0x0118e0, 0x016e60, 0x016e80, 0x01e922, 0x01e944, 
};
#define mxCharSet_Binary_Property_Dash 46
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Dash[mxCharSet_Binary_Property_Dash] = {
	0x00002d, 0x00002e, 0x00058a, 0x00058b, 0x0005be, 0x0005bf, 0x001400, 0x001401, 0x001806, 0x001807, 0x002010, 0x002016, 0x002053, 0x002054, 0x00207b, 0x00207c, 
	0x00208b, 0x00208c, 0x002212, 0x002213, 0x002e17, 0x002e18, 0x002e1a, 0x002e1b, 0x002e3a, 0x002e3c, 0x002e40, 0x002e41, 0x002e5d, 0x002e5e, 0x00301c, 0x00301d, 
	0x003030, 0x003031, 0x0030a0, 0x0030a1, 0x00fe31, 0x00fe33, 0x00fe58, 0x00fe59, 0x00fe63, 0x00fe64, 0x00ff0d, 0x00ff0e, 0x010ead, 0x010eae, 
};
#define mxCharSet_Binary_Property_Default_Ignorable_Code_Point 34
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Default_Ignorable_Code_Point[mxCharSet_Binary_Property_Default_Ignorable_Code_Point] = {
	0x0000ad, 0x0000ae, 0x00034f, 0x000350, 0x00061c, 0x00061d, 0x00115f, 0x001161, 0x0017b4, 0x0017b6, 0x00180b, 0x001810, 0x00200b, 0x002010, 0x00202a, 0x00202f, 
	0x002060, 0x002070, 0x003164, 0x003165, 0x00fe00, 0x00fe10, 0x00feff, 0x00ff00, 0x00ffa0, 0x00ffa1, 0x00fff0, 0x00fff9, 0x01bca0, 0x01bca4, 0x01d173, 0x01d17b, 
	0x0e0000, 0x0e1000, 
};
#define mxCharSet_Binary_Property_Deprecated 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Deprecated[mxCharSet_Binary_Property_Deprecated] = {
	0x000149, 0x00014a, 0x000673, 0x000674, 0x000f77, 0x000f78, 0x000f79, 0x000f7a, 0x0017a3, 0x0017a5, 0x00206a, 0x002070, 0x002329, 0x00232b, 0x0e0001, 0x0e0002, 
};
#define mxCharSet_Binary_Property_Diacritic 390
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Diacritic[mxCharSet_Binary_Property_Diacritic] = {
	0x00005e, 0x00005f, 0x000060, 0x000061, 0x0000a8, 0x0000a9, 0x0000af, 0x0000b0, 0x0000b4, 0x0000b5, 0x0000b7, 0x0000b9, 0x0002b0, 0x00034f, 0x000350, 0x000358, 
	0x00035d, 0x000363, 0x000374, 0x000376, 0x00037a, 0x00037b, 0x000384, 0x000386, 0x000483, 0x000488, 0x000559, 0x00055a, 0x000591, 0x0005a2, 0x0005a3, 0x0005be, 
	0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c5, 0x00064b, 0x000653, 0x000657, 0x000659, 0x0006df, 0x0006e1, 0x0006e5, 0x0006e7, 0x0006ea, 0x0006ed, 
	0x000730, 0x00074b, 0x0007a6, 0x0007b1, 0x0007eb, 0x0007f6, 0x000818, 0x00081a, 0x000898, 0x0008a0, 0x0008c9, 0x0008d3, 0x0008e3, 0x0008ff, 0x00093c, 0x00093d, 
	0x00094d, 0x00094e, 0x000951, 0x000955, 0x000971, 0x000972, 0x0009bc, 0x0009bd, 0x0009cd, 0x0009ce, 0x000a3c, 0x000a3d, 0x000a4d, 0x000a4e, 0x000abc, 0x000abd, 
	0x000acd, 0x000ace, 0x000afd, 0x000b00, 0x000b3c, 0x000b3d, 0x000b4d, 0x000b4e, 0x000b55, 0x000b56, 0x000bcd, 0x000bce, 0x000c3c, 0x000c3d, 0x000c4d, 0x000c4e, 
	0x000cbc, 0x000cbd, 0x000ccd, 0x000cce, 0x000d3b, 0x000d3d, 0x000d4d, 0x000d4e, 0x000dca, 0x000dcb, 0x000e47, 0x000e4d, 0x000e4e, 0x000e4f, 0x000eba, 0x000ebb, 
	0x000ec8, 0x000ecd, 0x000f18, 0x000f1a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 0x000f3e, 0x000f40, 0x000f82, 0x000f85, 0x000f86, 0x000f88, 
	0x000fc6, 0x000fc7, 0x001037, 0x001038, 0x001039, 0x00103b, 0x001063, 0x001065, 0x001069, 0x00106e, 0x001087, 0x00108e, 0x00108f, 0x001090, 0x00109a, 0x00109c, 
	0x00135d, 0x001360, 0x001714, 0x001716, 0x0017c9, 0x0017d4, 0x0017dd, 0x0017de, 0x001939, 0x00193c, 0x001a75, 0x001a7d, 0x001a7f, 0x001a80, 0x001ab0, 0x001abf, 
	0x001ac1, 0x001acc, 0x001b34, 0x001b35, 0x001b44, 0x001b45, 0x001b6b, 0x001b74, 0x001baa, 0x001bac, 0x001c36, 0x001c38, 0x001c78, 0x001c7e, 0x001cd0, 0x001ce9, 
	0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001d2c, 0x001d6b, 0x001dc4, 0x001dd0, 0x001df5, 0x001e00, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 
	0x001fcd, 0x001fd0, 0x001fdd, 0x001fe0, 0x001fed, 0x001ff0, 0x001ffd, 0x001fff, 0x002cef, 0x002cf2, 0x002e2f, 0x002e30, 0x00302a, 0x003030, 0x003099, 0x00309d, 
	0x0030fc, 0x0030fd, 0x00a66f, 0x00a670, 0x00a67c, 0x00a67e, 0x00a67f, 0x00a680, 0x00a69c, 0x00a69e, 0x00a6f0, 0x00a6f2, 0x00a700, 0x00a722, 0x00a788, 0x00a78b, 
	0x00a7f8, 0x00a7fa, 0x00a8c4, 0x00a8c5, 0x00a8e0, 0x00a8f2, 0x00a92b, 0x00a92f, 0x00a953, 0x00a954, 0x00a9b3, 0x00a9b4, 0x00a9c0, 0x00a9c1, 0x00a9e5, 0x00a9e6, 
	0x00aa7b, 0x00aa7e, 0x00aabf, 0x00aac3, 0x00aaf6, 0x00aaf7, 0x00ab5b, 0x00ab60, 0x00ab69, 0x00ab6c, 0x00abec, 0x00abee, 0x00fb1e, 0x00fb1f, 0x00fe20, 0x00fe30, 
	0x00ff3e, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ff70, 0x00ff71, 0x00ff9e, 0x00ffa0, 0x00ffe3, 0x00ffe4, 0x0102e0, 0x0102e1, 0x010780, 0x010786, 0x010787, 0x0107b1, 
	0x0107b2, 0x0107bb, 0x010ae5, 0x010ae7, 0x010d22, 0x010d28, 0x010efd, 0x010f00, 0x010f46, 0x010f51, 0x010f82, 0x010f86, 0x011046, 0x011047, 0x011070, 0x011071, 
	0x0110b9, 0x0110bb, 0x011133, 0x011135, 0x011173, 0x011174, 0x0111c0, 0x0111c1, 0x0111ca, 0x0111cd, 0x011235, 0x011237, 0x0112e9, 0x0112eb, 0x01133c, 0x01133d, 
	0x01134d, 0x01134e, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011442, 0x011443, 0x011446, 0x011447, 0x0114c2, 0x0114c4, 0x0115bf, 0x0115c1, 0x01163f, 0x011640, 
	0x0116b6, 0x0116b8, 0x01172b, 0x01172c, 0x011839, 0x01183b, 0x01193d, 0x01193f, 0x011943, 0x011944, 0x0119e0, 0x0119e1, 0x011a34, 0x011a35, 0x011a47, 0x011a48, 
	0x011a99, 0x011a9a, 0x011c3f, 0x011c40, 0x011d42, 0x011d43, 0x011d44, 0x011d46, 0x011d97, 0x011d98, 0x013447, 0x013456, 0x016af0, 0x016af5, 0x016b30, 0x016b37, 
	0x016f8f, 0x016fa0, 0x016ff0, 0x016ff2, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01d167, 0x01d16a, 
	0x01d16d, 0x01d173, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01e030, 0x01e06e, 0x01e130, 0x01e137, 0x01e2ae, 0x01e2af, 0x01e2ec, 0x01e2f0, 
	0x01e8d0, 0x01e8d7, 0x01e944, 0x01e947, 0x01e948, 0x01e94b, 
};
#define mxCharSet_Binary_Property_Emoji 302
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Emoji[mxCharSet_Binary_Property_Emoji] = {
	0x000023, 0x000024, 0x00002a, 0x00002b, 0x000030, 0x00003a, 0x0000a9, 0x0000aa, 0x0000ae, 0x0000af, 0x00203c, 0x00203d, 0x002049, 0x00204a, 0x002122, 0x002123, 
	0x002139, 0x00213a, 0x002194, 0x00219a, 0x0021a9, 0x0021ab, 0x00231a, 0x00231c, 0x002328, 0x002329, 0x0023cf, 0x0023d0, 0x0023e9, 0x0023f4, 0x0023f8, 0x0023fb, 
	0x0024c2, 0x0024c3, 0x0025aa, 0x0025ac, 0x0025b6, 0x0025b7, 0x0025c0, 0x0025c1, 0x0025fb, 0x0025ff, 0x002600, 0x002605, 0x00260e, 0x00260f, 0x002611, 0x002612, 
	0x002614, 0x002616, 0x002618, 0x002619, 0x00261d, 0x00261e, 0x002620, 0x002621, 0x002622, 0x002624, 0x002626, 0x002627, 0x00262a, 0x00262b, 0x00262e, 0x002630, 
	0x002638, 0x00263b, 0x002640, 0x002641, 0x002642, 0x002643, 0x002648, 0x002654, 0x00265f, 0x002661, 0x002663, 0x002664, 0x002665, 0x002667, 0x002668, 0x002669, 
	0x00267b, 0x00267c, 0x00267e, 0x002680, 0x002692, 0x002698, 0x002699, 0x00269a, 0x00269b, 0x00269d, 0x0026a0, 0x0026a2, 0x0026a7, 0x0026a8, 0x0026aa, 0x0026ac, 
	0x0026b0, 0x0026b2, 0x0026bd, 0x0026bf, 0x0026c4, 0x0026c6, 0x0026c8, 0x0026c9, 0x0026ce, 0x0026d0, 0x0026d1, 0x0026d2, 0x0026d3, 0x0026d5, 0x0026e9, 0x0026eb, 
	0x0026f0, 0x0026f6, 0x0026f7, 0x0026fb, 0x0026fd, 0x0026fe, 0x002702, 0x002703, 0x002705, 0x002706, 0x002708, 0x00270e, 0x00270f, 0x002710, 0x002712, 0x002713, 
	0x002714, 0x002715, 0x002716, 0x002717, 0x00271d, 0x00271e, 0x002721, 0x002722, 0x002728, 0x002729, 0x002733, 0x002735, 0x002744, 0x002745, 0x002747, 0x002748, 
	0x00274c, 0x00274d, 0x00274e, 0x00274f, 0x002753, 0x002756, 0x002757, 0x002758, 0x002763, 0x002765, 0x002795, 0x002798, 0x0027a1, 0x0027a2, 0x0027b0, 0x0027b1, 
	0x0027bf, 0x0027c0, 0x002934, 0x002936, 0x002b05, 0x002b08, 0x002b1b, 0x002b1d, 0x002b50, 0x002b51, 0x002b55, 0x002b56, 0x003030, 0x003031, 0x00303d, 0x00303e, 
	0x003297, 0x003298, 0x003299, 0x00329a, 0x01f004, 0x01f005, 0x01f0cf, 0x01f0d0, 0x01f170, 0x01f172, 0x01f17e, 0x01f180, 0x01f18e, 0x01f18f, 0x01f191, 0x01f19b, 
	0x01f1e6, 0x01f200, 0x01f201, 0x01f203, 0x01f21a, 0x01f21b, 0x01f22f, 0x01f230, 0x01f232, 0x01f23b, 0x01f250, 0x01f252, 0x01f300, 0x01f322, 0x01f324, 0x01f394, 
	0x01f396, 0x01f398, 0x01f399, 0x01f39c, 0x01f39e, 0x01f3f1, 0x01f3f3, 0x01f3f6, 0x01f3f7, 0x01f4fe, 0x01f4ff, 0x01f53e, 0x01f549, 0x01f54f, 0x01f550, 0x01f568, 
	0x01f56f, 0x01f571, 0x01f573, 0x01f57b, 0x01f587, 0x01f588, 0x01f58a, 0x01f58e, 0x01f590, 0x01f591, 0x01f595, 0x01f597, 0x01f5a4, 0x01f5a6, 0x01f5a8, 0x01f5a9, 
	0x01f5b1, 0x01f5b3, 0x01f5bc, 0x01f5bd, 0x01f5c2, 0x01f5c5, 0x01f5d1, 0x01f5d4, 0x01f5dc, 0x01f5df, 0x01f5e1, 0x01f5e2, 0x01f5e3, 0x01f5e4, 0x01f5e8, 0x01f5e9, 
	0x01f5ef, 0x01f5f0, 0x01f5f3, 0x01f5f4, 0x01f5fa, 0x01f650, 0x01f680, 0x01f6c6, 0x01f6cb, 0x01f6d3, 0x01f6d5, 0x01f6d8, 0x01f6dc, 0x01f6e6, 0x01f6e9, 0x01f6ea, 
	0x01f6eb, 0x01f6ed, 0x01f6f0, 0x01f6f1, 0x01f6f3, 0x01f6fd, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f90c, 0x01f93b, 0x01f93c, 0x01f946, 0x01f947, 0x01fa00, 
	0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 
};
#define mxCharSet_Binary_Property_Emoji_Component 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Emoji_Component[mxCharSet_Binary_Property_Emoji_Component] = {
	0x000023, 0x000024, 0x00002a, 0x00002b, 0x000030, 0x00003a, 0x00200d, 0x00200e, 0x0020e3, 0x0020e4, 0x00fe0f, 0x00fe10, 0x01f1e6, 0x01f200, 0x01f3fb, 0x01f400, 
	0x01f9b0, 0x01f9b4, 0x0e0020, 0x0e0080, 
};
#define mxCharSet_Binary_Property_Emoji_Modifier 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Emoji_Modifier[mxCharSet_Binary_Property_Emoji_Modifier] = {
	0x01f3fb, 0x01f400, 
};
#define mxCharSet_Binary_Property_Emoji_Modifier_Base 80
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Emoji_Modifier_Base[mxCharSet_Binary_Property_Emoji_Modifier_Base] = {
	0x00261d, 0x00261e, 0x0026f9, 0x0026fa, 0x00270a, 0x00270e, 0x01f385, 0x01f386, 0x01f3c2, 0x01f3c5, 0x01f3c7, 0x01f3c8, 0x01f3ca, 0x01f3cd, 0x01f442, 0x01f444, 
	0x01f446, 0x01f451, 0x01f466, 0x01f479, 0x01f47c, 0x01f47d, 0x01f481, 0x01f484, 0x01f485, 0x01f488, 0x01f48f, 0x01f490, 0x01f491, 0x01f492, 0x01f4aa, 0x01f4ab, 
	0x01f574, 0x01f576, 0x01f57a, 0x01f57b, 0x01f590, 0x01f591, 0x01f595, 0x01f597, 0x01f645, 0x01f648, 0x01f64b, 0x01f650, 0x01f6a3, 0x01f6a4, 0x01f6b4, 0x01f6b7, 
	0x01f6c0, 0x01f6c1, 0x01f6cc, 0x01f6cd, 0x01f90c, 0x01f90d, 0x01f90f, 0x01f910, 0x01f918, 0x01f920, 0x01f926, 0x01f927, 0x01f930, 0x01f93a, 0x01f93c, 0x01f93f, 
	0x01f977, 0x01f978, 0x01f9b5, 0x01f9b7, 0x01f9b8, 0x01f9ba, 0x01f9bb, 0x01f9bc, 0x01f9cd, 0x01f9d0, 0x01f9d1, 0x01f9de, 0x01fac3, 0x01fac6, 0x01faf0, 0x01faf9, 
};
#define mxCharSet_Binary_Property_Emoji_Presentation 162
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Emoji_Presentation[mxCharSet_Binary_Property_Emoji_Presentation] = {
	0x00231a, 0x00231c, 0x0023e9, 0x0023ed, 0x0023f0, 0x0023f1, 0x0023f3, 0x0023f4, 0x0025fd, 0x0025ff, 0x002614, 0x002616, 0x002648, 0x002654, 0x00267f, 0x002680, 
	0x002693, 0x002694, 0x0026a1, 0x0026a2, 0x0026aa, 0x0026ac, 0x0026bd, 0x0026bf, 0x0026c4, 0x0026c6, 0x0026ce, 0x0026cf, 0x0026d4, 0x0026d5, 0x0026ea, 0x0026eb, 
	0x0026f2, 0x0026f4, 0x0026f5, 0x0026f6, 0x0026fa, 0x0026fb, 0x0026fd, 0x0026fe, 0x002705, 0x002706, 0x00270a, 0x00270c, 0x002728, 0x002729, 0x00274c, 0x00274d, 
	0x00274e, 0x00274f, 0x002753, 0x002756, 0x002757, 0x002758, 0x002795, 0x002798, 0x0027b0, 0x0027b1, 0x0027bf, 0x0027c0, 0x002b1b, 0x002b1d, 0x002b50, 0x002b51, 
	0x002b55, 0x002b56, 0x01f004, 0x01f005, 0x01f0cf, 0x01f0d0, 0x01f18e, 0x01f18f, 0x01f191, 0x01f19b, 0x01f1e6, 0x01f200, 0x01f201, 0x01f202, 0x01f21a, 0x01f21b, 
	0x01f22f, 0x01f230, 0x01f232, 0x01f237, 0x01f238, 0x01f23b, 0x01f250, 0x01f252, 0x01f300, 0x01f321, 0x01f32d, 0x01f336, 0x01f337, 0x01f37d, 0x01f37e, 0x01f394, 
	0x01f3a0, 0x01f3cb, 0x01f3cf, 0x01f3d4, 0x01f3e0, 0x01f3f1, 0x01f3f4, 0x01f3f5, 0x01f3f8, 0x01f43f, 0x01f440, 0x01f441, 0x01f442, 0x01f4fd, 0x01f4ff, 0x01f53e, 
	0x01f54b, 0x01f54f, 0x01f550, 0x01f568, 0x01f57a, 0x01f57b, 0x01f595, 0x01f597, 0x01f5a4, 0x01f5a5, 0x01f5fb, 0x01f650, 0x01f680, 0x01f6c6, 0x01f6cc, 0x01f6cd, 
	0x01f6d0, 0x01f6d3, 0x01f6d5, 0x01f6d8, 0x01f6dc, 0x01f6e0, 0x01f6eb, 0x01f6ed, 0x01f6f4, 0x01f6fd, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f90c, 0x01f93b, 
	0x01f93c, 0x01f946, 0x01f947, 0x01fa00, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 
	0x01faf0, 0x01faf9, 
};
#define mxCharSet_Binary_Property_Extended_Pictographic 156
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Extended_Pictographic[mxCharSet_Binary_Property_Extended_Pictographic] = {
	0x0000a9, 0x0000aa, 0x0000ae, 0x0000af, 0x00203c, 0x00203d, 0x002049, 0x00204a, 0x002122, 0x002123, 0x002139, 0x00213a, 0x002194, 0x00219a, 0x0021a9, 0x0021ab, 
	0x00231a, 0x00231c, 0x002328, 0x002329, 0x002388, 0x002389, 0x0023cf, 0x0023d0, 0x0023e9, 0x0023f4, 0x0023f8, 0x0023fb, 0x0024c2, 0x0024c3, 0x0025aa, 0x0025ac, 
	0x0025b6, 0x0025b7, 0x0025c0, 0x0025c1, 0x0025fb, 0x0025ff, 0x002600, 0x002606, 0x002607, 0x002613, 0x002614, 0x002686, 0x002690, 0x002706, 0x002708, 0x002713, 
	0x002714, 0x002715, 0x002716, 0x002717, 0x00271d, 0x00271e, 0x002721, 0x002722, 0x002728, 0x002729, 0x002733, 0x002735, 0x002744, 0x002745, 0x002747, 0x002748, 
	0x00274c, 0x00274d, 0x00274e, 0x00274f, 0x002753, 0x002756, 0x002757, 0x002758, 0x002763, 0x002768, 0x002795, 0x002798, 0x0027a1, 0x0027a2, 0x0027b0, 0x0027b1, 
	0x0027bf, 0x0027c0, 0x002934, 0x002936, 0x002b05, 0x002b08, 0x002b1b, 0x002b1d, 0x002b50, 0x002b51, 0x002b55, 0x002b56, 0x003030, 0x003031, 0x00303d, 0x00303e, 
	0x003297, 0x003298, 0x003299, 0x00329a, 0x01f000, 0x01f100, 0x01f10d, 0x01f110, 0x01f12f, 0x01f130, 0x01f16c, 0x01f172, 0x01f17e, 0x01f180, 0x01f18e, 0x01f18f, 
	0x01f191, 0x01f19b, 0x01f1ad, 0x01f1e6, 0x01f201, 0x01f210, 0x01f21a, 0x01f21b, 0x01f22f, 0x01f230, 0x01f232, 0x01f23b, 0x01f23c, 0x01f240, 0x01f249, 0x01f3fb, 
	0x01f400, 0x01f53e, 0x01f546, 0x01f650, 0x01f680, 0x01f700, 0x01f774, 0x01f780, 0x01f7d5, 0x01f800, 0x01f80c, 0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 
	0x01f888, 0x01f890, 0x01f8ae, 0x01f900, 0x01f90c, 0x01f93b, 0x01f93c, 0x01f946, 0x01f947, 0x01fb00, 0x01fc00, 0x01fffe, 
};
#define mxCharSet_Binary_Property_Extender 66
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Extender[mxCharSet_Binary_Property_Extender] = {
	0x0000b7, 0x0000b8, 0x0002d0, 0x0002d2, 0x000640, 0x000641, 0x0007fa, 0x0007fb, 0x000b55, 0x000b56, 0x000e46, 0x000e47, 0x000ec6, 0x000ec7, 0x00180a, 0x00180b, 
	0x001843, 0x001844, 0x001aa7, 0x001aa8, 0x001c36, 0x001c37, 0x001c7b, 0x001c7c, 0x003005, 0x003006, 0x003031, 0x003036, 0x00309d, 0x00309f, 0x0030fc, 0x0030ff, 
	0x00a015, 0x00a016, 0x00a60c, 0x00a60d, 0x00a9cf, 0x00a9d0, 0x00a9e6, 0x00a9e7, 0x00aa70, 0x00aa71, 0x00aadd, 0x00aade, 0x00aaf3, 0x00aaf5, 0x00ff70, 0x00ff71, 
	0x010781, 0x010783, 0x01135d, 0x01135e, 0x0115c6, 0x0115c9, 0x011a98, 0x011a99, 0x016b42, 0x016b44, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe4, 0x01e13c, 0x01e13e, 
	0x01e944, 0x01e947, 
};
#define mxCharSet_Binary_Property_Grapheme_Base 1750
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Grapheme_Base[mxCharSet_Binary_Property_Grapheme_Base] = {
	0x000020, 0x00007f, 0x0000a0, 0x0000ad, 0x0000ae, 0x000300, 0x000370, 0x000378, 0x00037a, 0x000380, 0x000384, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 
	0x0003a3, 0x000483, 0x00048a, 0x000530, 0x000531, 0x000557, 0x000559, 0x00058b, 0x00058d, 0x000590, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 
	0x0005c6, 0x0005c7, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x000606, 0x000610, 0x00061b, 0x00061c, 0x00061d, 0x00064b, 0x000660, 0x000670, 0x000671, 0x0006d6, 
	0x0006de, 0x0006df, 0x0006e5, 0x0006e7, 0x0006e9, 0x0006ea, 0x0006ee, 0x00070e, 0x000710, 0x000711, 0x000712, 0x000730, 0x00074d, 0x0007a6, 0x0007b1, 0x0007b2, 
	0x0007c0, 0x0007eb, 0x0007f4, 0x0007fb, 0x0007fe, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x000830, 0x00083f, 0x000840, 0x000859, 
	0x00085e, 0x00085f, 0x000860, 0x00086b, 0x000870, 0x00088f, 0x0008a0, 0x0008ca, 0x000903, 0x00093a, 0x00093b, 0x00093c, 0x00093d, 0x000941, 0x000949, 0x00094d, 
	0x00094e, 0x000951, 0x000958, 0x000962, 0x000964, 0x000981, 0x000982, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 
	0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009be, 0x0009bf, 0x0009c1, 0x0009c7, 0x0009c9, 0x0009cb, 0x0009cd, 0x0009ce, 0x0009cf, 0x0009dc, 0x0009de, 
	0x0009df, 0x0009e2, 0x0009e6, 0x0009fe, 0x000a03, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 
	0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a3e, 0x000a41, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a70, 0x000a72, 0x000a75, 0x000a76, 0x000a77, 
	0x000a83, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000ac1, 
	0x000ac9, 0x000aca, 0x000acb, 0x000acd, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae2, 0x000ae6, 0x000af2, 0x000af9, 0x000afa, 0x000b02, 0x000b04, 0x000b05, 0x000b0d, 
	0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3d, 0x000b3e, 0x000b40, 0x000b41, 0x000b47, 0x000b49, 
	0x000b4b, 0x000b4d, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b62, 0x000b66, 0x000b78, 0x000b83, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 
	0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbf, 0x000bc0, 0x000bc1, 0x000bc3, 
	0x000bc6, 0x000bc9, 0x000bca, 0x000bcd, 0x000bd0, 0x000bd1, 0x000be6, 0x000bfb, 0x000c01, 0x000c04, 0x000c05, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 
	0x000c2a, 0x000c3a, 0x000c3d, 0x000c3e, 0x000c41, 0x000c45, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c62, 0x000c66, 0x000c70, 0x000c77, 0x000c81, 
	0x000c82, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cbf, 0x000cc0, 0x000cc2, 0x000cc3, 0x000cc5, 
	0x000cc7, 0x000cc9, 0x000cca, 0x000ccc, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce2, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x000d02, 0x000d0d, 0x000d0e, 0x000d11, 
	0x000d12, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d3f, 0x000d41, 0x000d46, 0x000d49, 0x000d4a, 0x000d4d, 0x000d4e, 0x000d50, 0x000d54, 0x000d57, 0x000d58, 0x000d62, 
	0x000d66, 0x000d80, 0x000d82, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dd0, 0x000dd2, 
	0x000dd8, 0x000ddf, 0x000de6, 0x000df0, 0x000df2, 0x000df5, 0x000e01, 0x000e31, 0x000e32, 0x000e34, 0x000e3f, 0x000e47, 0x000e4f, 0x000e5c, 0x000e81, 0x000e83, 
	0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ebe, 0x000ec0, 0x000ec5, 
	0x000ec6, 0x000ec7, 0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f18, 0x000f1a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 0x000f48, 
	0x000f49, 0x000f6d, 0x000f7f, 0x000f80, 0x000f85, 0x000f86, 0x000f88, 0x000f8d, 0x000fbe, 0x000fc6, 0x000fc7, 0x000fcd, 0x000fce, 0x000fdb, 0x001000, 0x00102d, 
	0x001031, 0x001032, 0x001038, 0x001039, 0x00103b, 0x00103d, 0x00103f, 0x001058, 0x00105a, 0x00105e, 0x001061, 0x001071, 0x001075, 0x001082, 0x001083, 0x001085, 
	0x001087, 0x00108d, 0x00108e, 0x00109d, 0x00109e, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 
	0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 
	0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001360, 0x00137d, 0x001380, 0x00139a, 0x0013a0, 0x0013f6, 
	0x0013f8, 0x0013fe, 0x001400, 0x00169d, 0x0016a0, 0x0016f9, 0x001700, 0x001712, 0x001715, 0x001716, 0x00171f, 0x001732, 0x001734, 0x001737, 0x001740, 0x001752, 
	0x001760, 0x00176d, 0x00176e, 0x001771, 0x001780, 0x0017b4, 0x0017b6, 0x0017b7, 0x0017be, 0x0017c6, 0x0017c7, 0x0017c9, 0x0017d4, 0x0017dd, 0x0017e0, 0x0017ea, 
	0x0017f0, 0x0017fa, 0x001800, 0x00180b, 0x001810, 0x00181a, 0x001820, 0x001879, 0x001880, 0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x0018ab, 0x0018b0, 0x0018f6, 
	0x001900, 0x00191f, 0x001923, 0x001927, 0x001929, 0x00192c, 0x001930, 0x001932, 0x001933, 0x001939, 0x001940, 0x001941, 0x001944, 0x00196e, 0x001970, 0x001975, 
	0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x001a17, 0x001a19, 0x001a1b, 0x001a1e, 0x001a56, 0x001a57, 0x001a58, 0x001a61, 0x001a62, 
	0x001a63, 0x001a65, 0x001a6d, 0x001a73, 0x001a80, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 0x001b04, 0x001b34, 0x001b3b, 0x001b3c, 0x001b3d, 0x001b42, 
	0x001b43, 0x001b4d, 0x001b50, 0x001b6b, 0x001b74, 0x001b7f, 0x001b82, 0x001ba2, 0x001ba6, 0x001ba8, 0x001baa, 0x001bab, 0x001bae, 0x001be6, 0x001be7, 0x001be8, 
	0x001bea, 0x001bed, 0x001bee, 0x001bef, 0x001bf2, 0x001bf4, 0x001bfc, 0x001c2c, 0x001c34, 0x001c36, 0x001c3b, 0x001c4a, 0x001c4d, 0x001c89, 0x001c90, 0x001cbb, 
	0x001cbd, 0x001cc8, 0x001cd3, 0x001cd4, 0x001ce1, 0x001ce2, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001cfb, 0x001d00, 0x001dc0, 
	0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 
	0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff6, 0x001fff, 
	0x002000, 0x00200b, 0x002010, 0x002028, 0x00202f, 0x002060, 0x002070, 0x002072, 0x002074, 0x00208f, 0x002090, 0x00209d, 0x0020a0, 0x0020c1, 0x002100, 0x00218c, 
	0x002190, 0x002427, 0x002440, 0x00244b, 0x002460, 0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002cef, 0x002cf2, 0x002cf4, 0x002cf9, 0x002d26, 0x002d27, 0x002d28, 
	0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d71, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 
	0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x002e00, 0x002e5e, 0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 
	0x002ff0, 0x002ffc, 0x003000, 0x00302a, 0x003030, 0x003040, 0x003041, 0x003097, 0x00309b, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x003190, 0x0031e4, 
	0x0031f0, 0x00321f, 0x003220, 0x00a48d, 0x00a490, 0x00a4c7, 0x00a4d0, 0x00a62c, 0x00a640, 0x00a66f, 0x00a673, 0x00a674, 0x00a67e, 0x00a69e, 0x00a6a0, 0x00a6f0, 
	0x00a6f2, 0x00a6f8, 0x00a700, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 
	0x00a80c, 0x00a825, 0x00a827, 0x00a82c, 0x00a830, 0x00a83a, 0x00a840, 0x00a878, 0x00a880, 0x00a8c4, 0x00a8ce, 0x00a8da, 0x00a8f2, 0x00a8ff, 0x00a900, 0x00a926, 
	0x00a92e, 0x00a947, 0x00a952, 0x00a954, 0x00a95f, 0x00a97d, 0x00a983, 0x00a9b3, 0x00a9b4, 0x00a9b6, 0x00a9ba, 0x00a9bc, 0x00a9be, 0x00a9ce, 0x00a9cf, 0x00a9da, 
	0x00a9de, 0x00a9e5, 0x00a9e6, 0x00a9ff, 0x00aa00, 0x00aa29, 0x00aa2f, 0x00aa31, 0x00aa33, 0x00aa35, 0x00aa40, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa4d, 0x00aa4e, 
	0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aa7c, 0x00aa7d, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 
	0x00aadb, 0x00aaec, 0x00aaee, 0x00aaf6, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab6c, 
	0x00ab70, 0x00abe5, 0x00abe6, 0x00abe8, 0x00abe9, 0x00abed, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 
	0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb1e, 0x00fb1f, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 
	0x00fb43, 0x00fb45, 0x00fb46, 0x00fbc3, 0x00fbd3, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe00, 0x00fe10, 0x00fe1a, 0x00fe30, 0x00fe53, 
	0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff01, 0x00ff9e, 0x00ffa0, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 
	0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fffc, 0x00fffe, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 
	0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x01018f, 0x010190, 0x01019d, 
	0x0101a0, 0x0101a1, 0x0101d0, 0x0101fd, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x0102e1, 0x0102fc, 0x010300, 0x010324, 0x01032d, 0x01034b, 0x010350, 0x010376, 
	0x010380, 0x01039e, 0x01039f, 0x0103c4, 0x0103c8, 0x0103d6, 0x010400, 0x01049e, 0x0104a0, 0x0104aa, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 
	0x010530, 0x010564, 0x01056f, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 
	0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 
	0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010857, 0x01089f, 0x0108a7, 0x0108b0, 0x0108e0, 0x0108f3, 
	0x0108f4, 0x0108f6, 0x0108fb, 0x01091c, 0x01091f, 0x01093a, 0x01093f, 0x010940, 0x010980, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a01, 0x010a10, 0x010a14, 
	0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a40, 0x010a49, 0x010a50, 0x010a59, 0x010a60, 0x010aa0, 0x010ac0, 0x010ae5, 0x010aeb, 0x010af7, 0x010b00, 0x010b36, 
	0x010b39, 0x010b56, 0x010b58, 0x010b73, 0x010b78, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 
	0x010cfa, 0x010d24, 0x010d30, 0x010d3a, 0x010e60, 0x010e7f, 0x010e80, 0x010eaa, 0x010ead, 0x010eae, 0x010eb0, 0x010eb2, 0x010f00, 0x010f28, 0x010f30, 0x010f46, 
	0x010f51, 0x010f5a, 0x010f70, 0x010f82, 0x010f86, 0x010f8a, 0x010fb0, 0x010fcc, 0x010fe0, 0x010ff7, 0x011000, 0x011001, 0x011002, 0x011038, 0x011047, 0x01104e, 
	0x011052, 0x011070, 0x011071, 0x011073, 0x011075, 0x011076, 0x011082, 0x0110b3, 0x0110b7, 0x0110b9, 0x0110bb, 0x0110bd, 0x0110be, 0x0110c2, 0x0110d0, 0x0110e9, 
	0x0110f0, 0x0110fa, 0x011103, 0x011127, 0x01112c, 0x01112d, 0x011136, 0x011148, 0x011150, 0x011173, 0x011174, 0x011177, 0x011182, 0x0111b6, 0x0111bf, 0x0111c9, 
	0x0111cd, 0x0111cf, 0x0111d0, 0x0111e0, 0x0111e1, 0x0111f5, 0x011200, 0x011212, 0x011213, 0x01122f, 0x011232, 0x011234, 0x011235, 0x011236, 0x011238, 0x01123e, 
	0x01123f, 0x011241, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 0x0112b0, 0x0112df, 0x0112e0, 0x0112e3, 
	0x0112f0, 0x0112fa, 0x011302, 0x011304, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 
	0x01133d, 0x01133e, 0x01133f, 0x011340, 0x011341, 0x011345, 0x011347, 0x011349, 0x01134b, 0x01134e, 0x011350, 0x011351, 0x01135d, 0x011364, 0x011400, 0x011438, 
	0x011440, 0x011442, 0x011445, 0x011446, 0x011447, 0x01145c, 0x01145d, 0x01145e, 0x01145f, 0x011462, 0x011480, 0x0114b0, 0x0114b1, 0x0114b3, 0x0114b9, 0x0114ba, 
	0x0114bb, 0x0114bd, 0x0114be, 0x0114bf, 0x0114c1, 0x0114c2, 0x0114c4, 0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115af, 0x0115b0, 0x0115b2, 0x0115b8, 0x0115bc, 
	0x0115be, 0x0115bf, 0x0115c1, 0x0115dc, 0x011600, 0x011633, 0x01163b, 0x01163d, 0x01163e, 0x01163f, 0x011641, 0x011645, 0x011650, 0x01165a, 0x011660, 0x01166d, 
	0x011680, 0x0116ab, 0x0116ac, 0x0116ad, 0x0116ae, 0x0116b0, 0x0116b6, 0x0116b7, 0x0116b8, 0x0116ba, 0x0116c0, 0x0116ca, 0x011700, 0x01171b, 0x011720, 0x011722, 
	0x011726, 0x011727, 0x011730, 0x011747, 0x011800, 0x01182f, 0x011838, 0x011839, 0x01183b, 0x01183c, 0x0118a0, 0x0118f3, 0x0118ff, 0x011907, 0x011909, 0x01190a, 
	0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011930, 0x011931, 0x011936, 0x011937, 0x011939, 0x01193d, 0x01193e, 0x01193f, 0x011943, 0x011944, 0x011947, 
	0x011950, 0x01195a, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d4, 0x0119dc, 0x0119e0, 0x0119e1, 0x0119e5, 0x011a00, 0x011a01, 0x011a0b, 0x011a33, 0x011a39, 0x011a3b, 
	0x011a3f, 0x011a47, 0x011a50, 0x011a51, 0x011a57, 0x011a59, 0x011a5c, 0x011a8a, 0x011a97, 0x011a98, 0x011a9a, 0x011aa3, 0x011ab0, 0x011af9, 0x011b00, 0x011b0a, 
	0x011c00, 0x011c09, 0x011c0a, 0x011c30, 0x011c3e, 0x011c3f, 0x011c40, 0x011c46, 0x011c50, 0x011c6d, 0x011c70, 0x011c90, 0x011ca9, 0x011caa, 0x011cb1, 0x011cb2, 
	0x011cb4, 0x011cb5, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d31, 0x011d46, 0x011d47, 0x011d50, 0x011d5a, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 
	0x011d6a, 0x011d8f, 0x011d93, 0x011d95, 0x011d96, 0x011d97, 0x011d98, 0x011d99, 0x011da0, 0x011daa, 0x011ee0, 0x011ef3, 0x011ef5, 0x011ef9, 0x011f02, 0x011f11, 
	0x011f12, 0x011f36, 0x011f3e, 0x011f40, 0x011f41, 0x011f42, 0x011f43, 0x011f5a, 0x011fb0, 0x011fb1, 0x011fc0, 0x011ff2, 0x011fff, 0x01239a, 0x012400, 0x01246f, 
	0x012470, 0x012475, 0x012480, 0x012544, 0x012f90, 0x012ff3, 0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 
	0x016a60, 0x016a6a, 0x016a6e, 0x016abf, 0x016ac0, 0x016aca, 0x016ad0, 0x016aee, 0x016af5, 0x016af6, 0x016b00, 0x016b30, 0x016b37, 0x016b46, 0x016b50, 0x016b5a, 
	0x016b5b, 0x016b62, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e9b, 0x016f00, 0x016f4b, 0x016f50, 0x016f88, 0x016f93, 0x016fa0, 0x016fe0, 0x016fe4, 
	0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 
	0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 
	0x01bc90, 0x01bc9a, 0x01bc9c, 0x01bc9d, 0x01bc9f, 0x01bca0, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d165, 0x01d166, 0x01d167, 
	0x01d16a, 0x01d16e, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d1eb, 0x01d200, 0x01d242, 0x01d245, 0x01d246, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 
	0x01d300, 0x01d357, 0x01d360, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 
	0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 
	0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01da00, 0x01da37, 0x01da3b, 
	0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da8c, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e030, 0x01e06e, 0x01e100, 0x01e12d, 0x01e137, 0x01e13e, 
	0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 0x01e290, 0x01e2ae, 0x01e2c0, 0x01e2ec, 0x01e2f0, 0x01e2fa, 0x01e2ff, 0x01e300, 0x01e4d0, 0x01e4ec, 0x01e4f0, 0x01e4fa, 
	0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e8c7, 0x01e8d0, 0x01e900, 0x01e944, 0x01e94b, 0x01e94c, 
	0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 0x01ec71, 0x01ecb5, 0x01ed01, 0x01ed3e, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 
	0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 
	0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 
	0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 
	0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 
	0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 0x01f0c1, 0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 
	0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 
	0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 
	0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 
	0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 0x01fbf0, 0x01fbfa, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 
	0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Binary_Property_Grapheme_Extend 726
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Grapheme_Extend[mxCharSet_Binary_Property_Grapheme_Extend] = {
	0x000300, 0x000370, 0x000483, 0x00048a, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x000610, 0x00061b, 
	0x00064b, 0x000660, 0x000670, 0x000671, 0x0006d6, 0x0006dd, 0x0006df, 0x0006e5, 0x0006e7, 0x0006e9, 0x0006ea, 0x0006ee, 0x000711, 0x000712, 0x000730, 0x00074b, 
	0x0007a6, 0x0007b1, 0x0007eb, 0x0007f4, 0x0007fd, 0x0007fe, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x00082e, 0x000859, 0x00085c, 
	0x000898, 0x0008a0, 0x0008ca, 0x0008e2, 0x0008e3, 0x000903, 0x00093a, 0x00093b, 0x00093c, 0x00093d, 0x000941, 0x000949, 0x00094d, 0x00094e, 0x000951, 0x000958, 
	0x000962, 0x000964, 0x000981, 0x000982, 0x0009bc, 0x0009bd, 0x0009be, 0x0009bf, 0x0009c1, 0x0009c5, 0x0009cd, 0x0009ce, 0x0009d7, 0x0009d8, 0x0009e2, 0x0009e4, 
	0x0009fe, 0x0009ff, 0x000a01, 0x000a03, 0x000a3c, 0x000a3d, 0x000a41, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a70, 0x000a72, 
	0x000a75, 0x000a76, 0x000a81, 0x000a83, 0x000abc, 0x000abd, 0x000ac1, 0x000ac6, 0x000ac7, 0x000ac9, 0x000acd, 0x000ace, 0x000ae2, 0x000ae4, 0x000afa, 0x000b00, 
	0x000b01, 0x000b02, 0x000b3c, 0x000b3d, 0x000b3e, 0x000b40, 0x000b41, 0x000b45, 0x000b4d, 0x000b4e, 0x000b55, 0x000b58, 0x000b62, 0x000b64, 0x000b82, 0x000b83, 
	0x000bbe, 0x000bbf, 0x000bc0, 0x000bc1, 0x000bcd, 0x000bce, 0x000bd7, 0x000bd8, 0x000c00, 0x000c01, 0x000c04, 0x000c05, 0x000c3c, 0x000c3d, 0x000c3e, 0x000c41, 
	0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c62, 0x000c64, 0x000c81, 0x000c82, 0x000cbc, 0x000cbd, 0x000cbf, 0x000cc0, 0x000cc2, 0x000cc3, 
	0x000cc6, 0x000cc7, 0x000ccc, 0x000cce, 0x000cd5, 0x000cd7, 0x000ce2, 0x000ce4, 0x000d00, 0x000d02, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d3f, 0x000d41, 0x000d45, 
	0x000d4d, 0x000d4e, 0x000d57, 0x000d58, 0x000d62, 0x000d64, 0x000d81, 0x000d82, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd0, 0x000dd2, 0x000dd5, 0x000dd6, 0x000dd7, 
	0x000ddf, 0x000de0, 0x000e31, 0x000e32, 0x000e34, 0x000e3b, 0x000e47, 0x000e4f, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ec8, 0x000ecf, 0x000f18, 0x000f1a, 
	0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 0x000f71, 0x000f7f, 0x000f80, 0x000f85, 0x000f86, 0x000f88, 0x000f8d, 0x000f98, 0x000f99, 0x000fbd, 
	0x000fc6, 0x000fc7, 0x00102d, 0x001031, 0x001032, 0x001038, 0x001039, 0x00103b, 0x00103d, 0x00103f, 0x001058, 0x00105a, 0x00105e, 0x001061, 0x001071, 0x001075, 
	0x001082, 0x001083, 0x001085, 0x001087, 0x00108d, 0x00108e, 0x00109d, 0x00109e, 0x00135d, 0x001360, 0x001712, 0x001715, 0x001732, 0x001734, 0x001752, 0x001754, 
	0x001772, 0x001774, 0x0017b4, 0x0017b6, 0x0017b7, 0x0017be, 0x0017c6, 0x0017c7, 0x0017c9, 0x0017d4, 0x0017dd, 0x0017de, 0x00180b, 0x00180e, 0x00180f, 0x001810, 
	0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x001920, 0x001923, 0x001927, 0x001929, 0x001932, 0x001933, 0x001939, 0x00193c, 0x001a17, 0x001a19, 0x001a1b, 0x001a1c, 
	0x001a56, 0x001a57, 0x001a58, 0x001a5f, 0x001a60, 0x001a61, 0x001a62, 0x001a63, 0x001a65, 0x001a6d, 0x001a73, 0x001a7d, 0x001a7f, 0x001a80, 0x001ab0, 0x001acf, 
	0x001b00, 0x001b04, 0x001b34, 0x001b3b, 0x001b3c, 0x001b3d, 0x001b42, 0x001b43, 0x001b6b, 0x001b74, 0x001b80, 0x001b82, 0x001ba2, 0x001ba6, 0x001ba8, 0x001baa, 
	0x001bab, 0x001bae, 0x001be6, 0x001be7, 0x001be8, 0x001bea, 0x001bed, 0x001bee, 0x001bef, 0x001bf2, 0x001c2c, 0x001c34, 0x001c36, 0x001c38, 0x001cd0, 0x001cd3, 
	0x001cd4, 0x001ce1, 0x001ce2, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001dc0, 0x001e00, 0x00200c, 0x00200d, 0x0020d0, 0x0020f1, 
	0x002cef, 0x002cf2, 0x002d7f, 0x002d80, 0x002de0, 0x002e00, 0x00302a, 0x003030, 0x003099, 0x00309b, 0x00a66f, 0x00a673, 0x00a674, 0x00a67e, 0x00a69e, 0x00a6a0, 
	0x00a6f0, 0x00a6f2, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a825, 0x00a827, 0x00a82c, 0x00a82d, 0x00a8c4, 0x00a8c6, 0x00a8e0, 0x00a8f2, 
	0x00a8ff, 0x00a900, 0x00a926, 0x00a92e, 0x00a947, 0x00a952, 0x00a980, 0x00a983, 0x00a9b3, 0x00a9b4, 0x00a9b6, 0x00a9ba, 0x00a9bc, 0x00a9be, 0x00a9e5, 0x00a9e6, 
	0x00aa29, 0x00aa2f, 0x00aa31, 0x00aa33, 0x00aa35, 0x00aa37, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa4d, 0x00aa7c, 0x00aa7d, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 
	0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aaec, 0x00aaee, 0x00aaf6, 0x00aaf7, 0x00abe5, 0x00abe6, 0x00abe8, 0x00abe9, 0x00abed, 0x00abee, 
	0x00fb1e, 0x00fb1f, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe30, 0x00ff9e, 0x00ffa0, 0x0101fd, 0x0101fe, 0x0102e0, 0x0102e1, 0x010376, 0x01037b, 0x010a01, 0x010a04, 
	0x010a05, 0x010a07, 0x010a0c, 0x010a10, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 0x010ae5, 0x010ae7, 0x010d24, 0x010d28, 0x010eab, 0x010ead, 0x010efd, 0x010f00, 
	0x010f46, 0x010f51, 0x010f82, 0x010f86, 0x011001, 0x011002, 0x011038, 0x011047, 0x011070, 0x011071, 0x011073, 0x011075, 0x01107f, 0x011082, 0x0110b3, 0x0110b7, 
	0x0110b9, 0x0110bb, 0x0110c2, 0x0110c3, 0x011100, 0x011103, 0x011127, 0x01112c, 0x01112d, 0x011135, 0x011173, 0x011174, 0x011180, 0x011182, 0x0111b6, 0x0111bf, 
	0x0111c9, 0x0111cd, 0x0111cf, 0x0111d0, 0x01122f, 0x011232, 0x011234, 0x011235, 0x011236, 0x011238, 0x01123e, 0x01123f, 0x011241, 0x011242, 0x0112df, 0x0112e0, 
	0x0112e3, 0x0112eb, 0x011300, 0x011302, 0x01133b, 0x01133d, 0x01133e, 0x01133f, 0x011340, 0x011341, 0x011357, 0x011358, 0x011366, 0x01136d, 0x011370, 0x011375, 
	0x011438, 0x011440, 0x011442, 0x011445, 0x011446, 0x011447, 0x01145e, 0x01145f, 0x0114b0, 0x0114b1, 0x0114b3, 0x0114b9, 0x0114ba, 0x0114bb, 0x0114bd, 0x0114be, 
	0x0114bf, 0x0114c1, 0x0114c2, 0x0114c4, 0x0115af, 0x0115b0, 0x0115b2, 0x0115b6, 0x0115bc, 0x0115be, 0x0115bf, 0x0115c1, 0x0115dc, 0x0115de, 0x011633, 0x01163b, 
	0x01163d, 0x01163e, 0x01163f, 0x011641, 0x0116ab, 0x0116ac, 0x0116ad, 0x0116ae, 0x0116b0, 0x0116b6, 0x0116b7, 0x0116b8, 0x01171d, 0x011720, 0x011722, 0x011726, 
	0x011727, 0x01172c, 0x01182f, 0x011838, 0x011839, 0x01183b, 0x011930, 0x011931, 0x01193b, 0x01193d, 0x01193e, 0x01193f, 0x011943, 0x011944, 0x0119d4, 0x0119d8, 
	0x0119da, 0x0119dc, 0x0119e0, 0x0119e1, 0x011a01, 0x011a0b, 0x011a33, 0x011a39, 0x011a3b, 0x011a3f, 0x011a47, 0x011a48, 0x011a51, 0x011a57, 0x011a59, 0x011a5c, 
	0x011a8a, 0x011a97, 0x011a98, 0x011a9a, 0x011c30, 0x011c37, 0x011c38, 0x011c3e, 0x011c3f, 0x011c40, 0x011c92, 0x011ca8, 0x011caa, 0x011cb1, 0x011cb2, 0x011cb4, 
	0x011cb5, 0x011cb7, 0x011d31, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d46, 0x011d47, 0x011d48, 0x011d90, 0x011d92, 0x011d95, 0x011d96, 
	0x011d97, 0x011d98, 0x011ef3, 0x011ef5, 0x011f00, 0x011f02, 0x011f36, 0x011f3b, 0x011f40, 0x011f41, 0x011f42, 0x011f43, 0x013440, 0x013441, 0x013447, 0x013456, 
	0x016af0, 0x016af5, 0x016b30, 0x016b37, 0x016f4f, 0x016f50, 0x016f8f, 0x016f93, 0x016fe4, 0x016fe5, 0x01bc9d, 0x01bc9f, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 
	0x01d165, 0x01d166, 0x01d167, 0x01d16a, 0x01d16e, 0x01d173, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 0x01da00, 0x01da37, 
	0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 
	0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e08f, 0x01e090, 0x01e130, 0x01e137, 0x01e2ae, 0x01e2af, 0x01e2ec, 0x01e2f0, 0x01e4ec, 0x01e4f0, 0x01e8d0, 0x01e8d7, 
	0x01e944, 0x01e94b, 0x0e0020, 0x0e0080, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Binary_Property_Hex_Digit 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Hex_Digit[mxCharSet_Binary_Property_Hex_Digit] = {
	0x000030, 0x00003a, 0x000041, 0x000047, 0x000061, 0x000067, 0x00ff10, 0x00ff1a, 0x00ff21, 0x00ff27, 0x00ff41, 0x00ff47, 
};
#define mxCharSet_Binary_Property_IDS_Binary_Operator 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_IDS_Binary_Operator[mxCharSet_Binary_Property_IDS_Binary_Operator] = {
	0x002ff0, 0x002ff2, 0x002ff4, 0x002ffc, 
};
#define mxCharSet_Binary_Property_IDS_Trinary_Operator 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_IDS_Trinary_Operator[mxCharSet_Binary_Property_IDS_Trinary_Operator] = {
	0x002ff2, 0x002ff4, 
};
#define mxCharSet_Binary_Property_ID_Continue 1536
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_ID_Continue[mxCharSet_Binary_Property_ID_Continue] = {
	0x000030, 0x00003a, 0x000041, 0x00005b, 0x00005f, 0x000060, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000b7, 0x0000b8, 0x0000ba, 0x0000bb, 
	0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000300, 0x000375, 
	0x000376, 0x000378, 0x00037a, 0x00037e, 0x00037f, 0x000380, 0x000386, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 
	0x000483, 0x000488, 0x00048a, 0x000530, 0x000531, 0x000557, 0x000559, 0x00055a, 0x000560, 0x000589, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 
	0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000610, 0x00061b, 0x000620, 0x00066a, 0x00066e, 0x0006d4, 0x0006d5, 0x0006dd, 
	0x0006df, 0x0006e9, 0x0006ea, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x00074b, 0x00074d, 0x0007b2, 0x0007c0, 0x0007f6, 0x0007fa, 0x0007fb, 0x0007fd, 0x0007fe, 
	0x000800, 0x00082e, 0x000840, 0x00085c, 0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x000898, 0x0008e2, 0x0008e3, 0x000964, 0x000966, 0x000970, 
	0x000971, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 
	0x0009c7, 0x0009c9, 0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009f2, 0x0009fc, 0x0009fd, 0x0009fe, 0x0009ff, 
	0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 
	0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a76, 
	0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 
	0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 0x000ae6, 0x000af0, 0x000af9, 0x000b00, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 
	0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 
	0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 0x000b70, 0x000b71, 0x000b72, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 
	0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 
	0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000be6, 0x000bf0, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 
	0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 
	0x000c66, 0x000c70, 0x000c80, 0x000c84, 0x000c85, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 
	0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 
	0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d4f, 0x000d54, 0x000d58, 0x000d5f, 0x000d64, 0x000d66, 0x000d70, 0x000d7a, 0x000d80, 
	0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd5, 
	0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df4, 0x000e01, 0x000e3b, 0x000e40, 0x000e4f, 0x000e50, 0x000e5a, 0x000e81, 0x000e83, 
	0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 
	0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f01, 0x000f18, 0x000f1a, 0x000f20, 0x000f2a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 
	0x000f3e, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f85, 0x000f86, 0x000f98, 0x000f99, 0x000fbd, 0x000fc6, 0x000fc7, 0x001000, 0x00104a, 0x001050, 0x00109e, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 
	0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 
	0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x00135d, 0x001360, 0x001369, 0x001372, 0x001380, 0x001390, 0x0013a0, 0x0013f6, 
	0x0013f8, 0x0013fe, 0x001401, 0x00166d, 0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 0x001700, 0x001716, 0x00171f, 0x001735, 
	0x001740, 0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017d4, 0x0017d7, 0x0017d8, 0x0017dc, 0x0017de, 0x0017e0, 0x0017ea, 
	0x00180b, 0x00180e, 0x00180f, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x00193c, 
	0x001946, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x001a00, 0x001a1c, 0x001a20, 0x001a5f, 0x001a60, 0x001a7d, 
	0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa7, 0x001aa8, 0x001ab0, 0x001abe, 0x001abf, 0x001acf, 0x001b00, 0x001b4d, 0x001b50, 0x001b5a, 0x001b6b, 0x001b74, 
	0x001b80, 0x001bf4, 0x001c00, 0x001c38, 0x001c40, 0x001c4a, 0x001c4d, 0x001c7e, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001cd0, 0x001cd3, 
	0x001cd4, 0x001cfb, 0x001d00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 
	0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 
	0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x00203f, 0x002041, 0x002054, 0x002055, 0x002071, 0x002072, 0x00207f, 0x002080, 
	0x002090, 0x00209d, 0x0020d0, 0x0020dd, 0x0020e1, 0x0020e2, 0x0020e5, 0x0020f1, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 
	0x002118, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 
	0x002160, 0x002189, 0x002c00, 0x002ce5, 0x002ceb, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d70, 
	0x002d7f, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 
	0x002dd8, 0x002ddf, 0x002de0, 0x002e00, 0x003005, 0x003008, 0x003021, 0x003030, 0x003031, 0x003036, 0x003038, 0x00303d, 0x003041, 0x003097, 0x003099, 0x0030a0, 
	0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 0x003400, 0x004dc0, 0x004e00, 0x00a48d, 
	0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a62c, 0x00a640, 0x00a670, 0x00a674, 0x00a67e, 0x00a67f, 0x00a6f2, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 
	0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a828, 0x00a82c, 0x00a82d, 0x00a840, 0x00a874, 0x00a880, 0x00a8c6, 
	0x00a8d0, 0x00a8da, 0x00a8e0, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a92e, 0x00a930, 0x00a954, 0x00a960, 0x00a97d, 0x00a980, 0x00a9c1, 0x00a9cf, 0x00a9da, 
	0x00a9e0, 0x00a9ff, 0x00aa00, 0x00aa37, 0x00aa40, 0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aac3, 0x00aadb, 0x00aade, 0x00aae0, 0x00aaf0, 
	0x00aaf2, 0x00aaf7, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 
	0x00ab70, 0x00abeb, 0x00abec, 0x00abee, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 
	0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb29, 0x00fb2a, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 
	0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 0x00fd50, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfc, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe30, 0x00fe33, 0x00fe35, 
	0x00fe4d, 0x00fe50, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff10, 0x00ff1a, 0x00ff21, 0x00ff3b, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ffbf, 
	0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 
	0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010140, 0x010175, 0x0101fd, 0x0101fe, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x0102e0, 0x0102e1, 
	0x010300, 0x010320, 0x01032d, 0x01034b, 0x010350, 0x01037b, 0x010380, 0x01039e, 0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x0103d1, 0x0103d6, 0x010400, 0x01049e, 
	0x0104a0, 0x0104aa, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 
	0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 
	0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 
	0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 
	0x0109be, 0x0109c0, 0x010a00, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 
	0x010a60, 0x010a7d, 0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae7, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 0x010b60, 0x010b73, 0x010b80, 0x010b92, 
	0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d28, 0x010d30, 0x010d3a, 0x010e80, 0x010eaa, 0x010eab, 0x010ead, 0x010eb0, 0x010eb2, 
	0x010efd, 0x010f1d, 0x010f27, 0x010f28, 0x010f30, 0x010f51, 0x010f70, 0x010f86, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011000, 0x011047, 0x011066, 0x011076, 
	0x01107f, 0x0110bb, 0x0110c2, 0x0110c3, 0x0110d0, 0x0110e9, 0x0110f0, 0x0110fa, 0x011100, 0x011135, 0x011136, 0x011140, 0x011144, 0x011148, 0x011150, 0x011174, 
	0x011176, 0x011177, 0x011180, 0x0111c5, 0x0111c9, 0x0111cd, 0x0111ce, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x011238, 0x01123e, 0x011242, 
	0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 0x011300, 0x011304, 
	0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133b, 0x011345, 0x011347, 0x011349, 
	0x01134b, 0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011400, 0x01144b, 0x011450, 0x01145a, 
	0x01145e, 0x011462, 0x011480, 0x0114c6, 0x0114c7, 0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115b6, 0x0115b8, 0x0115c1, 0x0115d8, 0x0115de, 0x011600, 0x011641, 
	0x011644, 0x011645, 0x011650, 0x01165a, 0x011680, 0x0116b9, 0x0116c0, 0x0116ca, 0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x01173a, 0x011740, 0x011747, 
	0x011800, 0x01183b, 0x0118a0, 0x0118ea, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 
	0x01193b, 0x011944, 0x011950, 0x01195a, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 0x0119e2, 0x0119e3, 0x0119e5, 0x011a00, 0x011a3f, 0x011a47, 0x011a48, 
	0x011a50, 0x011a9a, 0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c41, 0x011c50, 0x011c5a, 0x011c72, 0x011c90, 
	0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 
	0x011d50, 0x011d5a, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 0x011daa, 0x011ee0, 0x011ef7, 
	0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f43, 0x011f50, 0x011f5a, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 0x012400, 0x01246f, 0x012480, 0x012544, 
	0x012f90, 0x012ff1, 0x013000, 0x013430, 0x013440, 0x013456, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a70, 0x016abf, 
	0x016ac0, 0x016aca, 0x016ad0, 0x016aee, 0x016af0, 0x016af5, 0x016b00, 0x016b37, 0x016b40, 0x016b44, 0x016b50, 0x016b5a, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 
	0x016e40, 0x016e80, 0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe5, 0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 
	0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 
	0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9d, 0x01bc9f, 
	0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01d165, 0x01d16a, 0x01d16d, 0x01d173, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 
	0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 
	0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 
	0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 
	0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01d7ce, 0x01d800, 0x01da00, 0x01da37, 
	0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 0x01e007, 
	0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 
	0x01e140, 0x01e14a, 0x01e14e, 0x01e14f, 0x01e290, 0x01e2af, 0x01e2c0, 0x01e2fa, 0x01e4d0, 0x01e4fa, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 
	0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e8d0, 0x01e8d7, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 
	0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 
	0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 
	0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 
	0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01fbf0, 0x01fbfa, 0x020000, 0x02a6e0, 
	0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Binary_Property_ID_Start 1318
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_ID_Start[mxCharSet_Binary_Property_ID_Start] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 
	0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000370, 0x000375, 0x000376, 0x000378, 0x00037a, 0x00037e, 0x00037f, 0x000380, 
	0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 0x00048a, 0x000530, 0x000531, 0x000557, 
	0x000559, 0x00055a, 0x000560, 0x000589, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000620, 0x00064b, 0x00066e, 0x000670, 0x000671, 0x0006d4, 0x0006d5, 0x0006d6, 
	0x0006e5, 0x0006e7, 0x0006ee, 0x0006f0, 0x0006fa, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x000711, 0x000712, 0x000730, 0x00074d, 0x0007a6, 0x0007b1, 0x0007b2, 
	0x0007ca, 0x0007eb, 0x0007f4, 0x0007f6, 0x0007fa, 0x0007fb, 0x000800, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x000840, 0x000859, 
	0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x0008a0, 0x0008ca, 0x000904, 0x00093a, 0x00093d, 0x00093e, 0x000950, 0x000951, 0x000958, 0x000962, 
	0x000971, 0x000981, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009be, 
	0x0009ce, 0x0009cf, 0x0009dc, 0x0009de, 0x0009df, 0x0009e2, 0x0009f0, 0x0009f2, 0x0009fc, 0x0009fd, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 
	0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a72, 0x000a75, 0x000a85, 0x000a8e, 
	0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000abe, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae2, 
	0x000af9, 0x000afa, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3d, 0x000b3e, 
	0x000b5c, 0x000b5e, 0x000b5f, 0x000b62, 0x000b71, 0x000b72, 0x000b83, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 
	0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bd0, 0x000bd1, 0x000c05, 0x000c0d, 0x000c0e, 0x000c11, 
	0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3d, 0x000c3e, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c62, 0x000c80, 0x000c81, 0x000c85, 0x000c8d, 
	0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cbe, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce2, 0x000cf1, 0x000cf3, 
	0x000d04, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d4e, 0x000d4f, 0x000d54, 0x000d57, 0x000d5f, 0x000d62, 0x000d7a, 0x000d80, 
	0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000e01, 0x000e31, 0x000e32, 0x000e34, 0x000e40, 0x000e47, 
	0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ebe, 
	0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000edc, 0x000ee0, 0x000f00, 0x000f01, 0x000f40, 0x000f48, 0x000f49, 0x000f6d, 0x000f88, 0x000f8d, 0x001000, 0x00102b, 
	0x00103f, 0x001040, 0x001050, 0x001056, 0x00105a, 0x00105e, 0x001061, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001071, 0x001075, 0x001082, 0x00108e, 0x00108f, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 
	0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 
	0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001380, 0x001390, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001401, 0x00166d, 
	0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 0x001700, 0x001712, 0x00171f, 0x001732, 0x001740, 0x001752, 0x001760, 0x00176d, 
	0x00176e, 0x001771, 0x001780, 0x0017b4, 0x0017d7, 0x0017d8, 0x0017dc, 0x0017dd, 0x001820, 0x001879, 0x001880, 0x0018a9, 0x0018aa, 0x0018ab, 0x0018b0, 0x0018f6, 
	0x001900, 0x00191f, 0x001950, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x001a00, 0x001a17, 0x001a20, 0x001a55, 0x001aa7, 0x001aa8, 
	0x001b05, 0x001b34, 0x001b45, 0x001b4d, 0x001b83, 0x001ba1, 0x001bae, 0x001bb0, 0x001bba, 0x001be6, 0x001c00, 0x001c24, 0x001c4d, 0x001c50, 0x001c5a, 0x001c7e, 
	0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001cfb, 0x001d00, 0x001dc0, 
	0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 
	0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 
	0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x002102, 0x002103, 0x002107, 0x002108, 
	0x00210a, 0x002114, 0x002115, 0x002116, 0x002118, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00213a, 0x00213c, 0x002140, 
	0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002160, 0x002189, 0x002c00, 0x002ce5, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 
	0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d70, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 
	0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x003005, 0x003008, 0x003021, 0x00302a, 0x003031, 0x003036, 0x003038, 0x00303d, 
	0x003041, 0x003097, 0x00309b, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 
	0x003400, 0x004dc0, 0x004e00, 0x00a48d, 0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a620, 0x00a62a, 0x00a62c, 0x00a640, 0x00a66f, 0x00a67f, 0x00a69e, 
	0x00a6a0, 0x00a6f0, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a802, 
	0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a823, 0x00a840, 0x00a874, 0x00a882, 0x00a8b4, 0x00a8f2, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a8ff, 
	0x00a90a, 0x00a926, 0x00a930, 0x00a947, 0x00a960, 0x00a97d, 0x00a984, 0x00a9b3, 0x00a9cf, 0x00a9d0, 0x00a9e0, 0x00a9e5, 0x00a9e6, 0x00a9f0, 0x00a9fa, 0x00a9ff, 
	0x00aa00, 0x00aa29, 0x00aa40, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aa7b, 0x00aa7e, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 
	0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 0x00aadb, 0x00aade, 0x00aae0, 0x00aaeb, 0x00aaf2, 0x00aaf5, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 
	0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abe3, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 
	0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb1e, 0x00fb1f, 0x00fb29, 0x00fb2a, 0x00fb37, 
	0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 0x00fd50, 0x00fd90, 0x00fd92, 0x00fdc8, 
	0x00fdf0, 0x00fdfc, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 
	0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 
	0x010080, 0x0100fb, 0x010140, 0x010175, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x010300, 0x010320, 0x01032d, 0x01034b, 0x010350, 0x010376, 0x010380, 0x01039e, 
	0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x0103d1, 0x0103d6, 0x010400, 0x01049e, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 
	0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 
	0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 
	0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 
	0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 0x010a00, 0x010a01, 0x010a10, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 
	0x010a60, 0x010a7d, 0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae5, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 0x010b60, 0x010b73, 0x010b80, 0x010b92, 
	0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d24, 0x010e80, 0x010eaa, 0x010eb0, 0x010eb2, 0x010f00, 0x010f1d, 0x010f27, 0x010f28, 
	0x010f30, 0x010f46, 0x010f70, 0x010f82, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011003, 0x011038, 0x011071, 0x011073, 0x011075, 0x011076, 0x011083, 0x0110b0, 
	0x0110d0, 0x0110e9, 0x011103, 0x011127, 0x011144, 0x011145, 0x011147, 0x011148, 0x011150, 0x011173, 0x011176, 0x011177, 0x011183, 0x0111b3, 0x0111c1, 0x0111c5, 
	0x0111da, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x01122c, 0x01123f, 0x011241, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 
	0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112df, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 
	0x011335, 0x01133a, 0x01133d, 0x01133e, 0x011350, 0x011351, 0x01135d, 0x011362, 0x011400, 0x011435, 0x011447, 0x01144b, 0x01145f, 0x011462, 0x011480, 0x0114b0, 
	0x0114c4, 0x0114c6, 0x0114c7, 0x0114c8, 0x011580, 0x0115af, 0x0115d8, 0x0115dc, 0x011600, 0x011630, 0x011644, 0x011645, 0x011680, 0x0116ab, 0x0116b8, 0x0116b9, 
	0x011700, 0x01171b, 0x011740, 0x011747, 0x011800, 0x01182c, 0x0118a0, 0x0118e0, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 
	0x011918, 0x011930, 0x01193f, 0x011940, 0x011941, 0x011942, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d1, 0x0119e1, 0x0119e2, 0x0119e3, 0x0119e4, 0x011a00, 0x011a01, 
	0x011a0b, 0x011a33, 0x011a3a, 0x011a3b, 0x011a50, 0x011a51, 0x011a5c, 0x011a8a, 0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c2f, 
	0x011c40, 0x011c41, 0x011c72, 0x011c90, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d31, 0x011d46, 0x011d47, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 
	0x011d6a, 0x011d8a, 0x011d98, 0x011d99, 0x011ee0, 0x011ef3, 0x011f02, 0x011f03, 0x011f04, 0x011f11, 0x011f12, 0x011f34, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 
	0x012400, 0x01246f, 0x012480, 0x012544, 0x012f90, 0x012ff1, 0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 
	0x016a70, 0x016abf, 0x016ad0, 0x016aee, 0x016b00, 0x016b30, 0x016b40, 0x016b44, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e80, 0x016f00, 0x016f4b, 
	0x016f50, 0x016f51, 0x016f93, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe4, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 
	0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 
	0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 
	0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 
	0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 
	0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 
	0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e030, 0x01e06e, 0x01e100, 0x01e12d, 0x01e137, 0x01e13e, 0x01e14e, 0x01e14f, 
	0x01e290, 0x01e2ae, 0x01e2c0, 0x01e2ec, 0x01e4d0, 0x01e4ec, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 
	0x01e900, 0x01e944, 0x01e94b, 0x01e94c, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 
	0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 
	0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 
	0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 
	0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 
	0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Binary_Property_Ideographic 40
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Ideographic[mxCharSet_Binary_Property_Ideographic] = {
	0x003006, 0x003008, 0x003021, 0x00302a, 0x003038, 0x00303b, 0x003400, 0x004dc0, 0x004e00, 0x00a000, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x016fe4, 0x016fe5, 
	0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01b170, 0x01b2fc, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 
	0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Binary_Property_Join_Control 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Join_Control[mxCharSet_Binary_Property_Join_Control] = {
	0x00200c, 0x00200e, 
};
#define mxCharSet_Binary_Property_Logical_Order_Exception 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Logical_Order_Exception[mxCharSet_Binary_Property_Logical_Order_Exception] = {
	0x000e40, 0x000e45, 0x000ec0, 0x000ec5, 0x0019b5, 0x0019b8, 0x0019ba, 0x0019bb, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aaba, 0x00aabb, 0x00aabd, 
};
#define mxCharSet_Binary_Property_Lowercase 1342
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Lowercase[mxCharSet_Binary_Property_Lowercase] = {
	0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000df, 0x0000f7, 0x0000f8, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 
	0x000105, 0x000106, 0x000107, 0x000108, 0x000109, 0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 
	0x000115, 0x000116, 0x000117, 0x000118, 0x000119, 0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 
	0x000125, 0x000126, 0x000127, 0x000128, 0x000129, 0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 
	0x000135, 0x000136, 0x000137, 0x000139, 0x00013a, 0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 
	0x000146, 0x000147, 0x000148, 0x00014a, 0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 
	0x000157, 0x000158, 0x000159, 0x00015a, 0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 
	0x000167, 0x000168, 0x000169, 0x00016a, 0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 
	0x000177, 0x000178, 0x00017a, 0x00017b, 0x00017c, 0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 
	0x000192, 0x000193, 0x000195, 0x000196, 0x000199, 0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 
	0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bb, 0x0001bd, 0x0001c0, 0x0001c6, 0x0001c7, 
	0x0001c9, 0x0001ca, 0x0001cc, 0x0001cd, 0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 
	0x0001da, 0x0001db, 0x0001dc, 0x0001de, 0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 
	0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 0x0001f1, 0x0001f3, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 
	0x0001ff, 0x000200, 0x000201, 0x000202, 0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 
	0x00020f, 0x000210, 0x000211, 0x000212, 0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 
	0x00021f, 0x000220, 0x000221, 0x000222, 0x000223, 0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 
	0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 
	0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 0x000294, 0x000295, 0x0002b9, 0x0002c0, 0x0002c2, 0x0002e0, 0x0002e5, 0x000345, 0x000346, 0x000371, 0x000372, 
	0x000373, 0x000374, 0x000377, 0x000378, 0x00037a, 0x00037e, 0x000390, 0x000391, 0x0003ac, 0x0003cf, 0x0003d0, 0x0003d2, 0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 
	0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 
	0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f4, 0x0003f5, 0x0003f6, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 
	0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 
	0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x000482, 
	0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 
	0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 
	0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 
	0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 
	0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 
	0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 
	0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 
	0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 
	0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 
	0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 
	0x00052d, 0x00052e, 0x00052f, 0x000530, 0x000560, 0x000589, 0x0010d0, 0x0010fb, 0x0010fc, 0x001100, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001d00, 0x001dc0, 
	0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 
	0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 
	0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 
	0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 
	0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 
	0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 
	0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 
	0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 
	0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 
	0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 
	0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 
	0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 
	0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 
	0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 
	0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 
	0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f16, 0x001f20, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f46, 
	0x001f50, 0x001f58, 0x001f60, 0x001f68, 0x001f70, 0x001f7e, 0x001f80, 0x001f88, 0x001f90, 0x001f98, 0x001fa0, 0x001fa8, 0x001fb0, 0x001fb5, 0x001fb6, 0x001fb8, 
	0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fc8, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fd8, 0x001fe0, 0x001fe8, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ff8, 
	0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x00210a, 0x00210b, 0x00210e, 0x002110, 0x002113, 0x002114, 0x00212f, 0x002130, 0x002134, 0x002135, 
	0x002139, 0x00213a, 0x00213c, 0x00213e, 0x002146, 0x00214a, 0x00214e, 0x00214f, 0x002170, 0x002180, 0x002184, 0x002185, 0x0024d0, 0x0024ea, 0x002c30, 0x002c60, 
	0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7e, 
	0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 
	0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 
	0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 
	0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 
	0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 
	0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 
	0x002ce1, 0x002ce2, 0x002ce3, 0x002ce5, 0x002cec, 0x002ced, 0x002cee, 0x002cef, 0x002cf3, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 
	0x00a641, 0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 
	0x00a651, 0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 
	0x00a661, 0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a66e, 0x00a681, 0x00a682, 
	0x00a683, 0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 
	0x00a693, 0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a69e, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 
	0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 
	0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 
	0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 
	0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 
	0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 
	0x00a785, 0x00a786, 0x00a787, 0x00a788, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a78f, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 
	0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 
	0x00a7af, 0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 
	0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7cb, 0x00a7d1, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7da, 
	0x00a7f2, 0x00a7f5, 0x00a7f6, 0x00a7f7, 0x00a7f8, 0x00a7fb, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 
	0x00ff41, 0x00ff5b, 0x010428, 0x010450, 0x0104d8, 0x0104fc, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010780, 0x010781, 
	0x010783, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010cc0, 0x010cf3, 0x0118c0, 0x0118e0, 0x016e60, 0x016e80, 0x01d41a, 0x01d434, 0x01d44e, 0x01d455, 
	0x01d456, 0x01d468, 0x01d482, 0x01d49c, 0x01d4b6, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d4d0, 0x01d4ea, 0x01d504, 0x01d51e, 0x01d538, 
	0x01d552, 0x01d56c, 0x01d586, 0x01d5a0, 0x01d5ba, 0x01d5d4, 0x01d5ee, 0x01d608, 0x01d622, 0x01d63c, 0x01d656, 0x01d670, 0x01d68a, 0x01d6a6, 0x01d6c2, 0x01d6db, 
	0x01d6dc, 0x01d6e2, 0x01d6fc, 0x01d715, 0x01d716, 0x01d71c, 0x01d736, 0x01d74f, 0x01d750, 0x01d756, 0x01d770, 0x01d789, 0x01d78a, 0x01d790, 0x01d7aa, 0x01d7c3, 
	0x01d7c4, 0x01d7ca, 0x01d7cb, 0x01d7cc, 0x01df00, 0x01df0a, 0x01df0b, 0x01df1f, 0x01df25, 0x01df2b, 0x01e030, 0x01e06e, 0x01e922, 0x01e944, 
};
#define mxCharSet_Binary_Property_Math 276
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Math[mxCharSet_Binary_Property_Math] = {
	0x00002b, 0x00002c, 0x00003c, 0x00003f, 0x00005e, 0x00005f, 0x00007c, 0x00007d, 0x00007e, 0x00007f, 0x0000ac, 0x0000ad, 0x0000b1, 0x0000b2, 0x0000d7, 0x0000d8, 
	0x0000f7, 0x0000f8, 0x0003d0, 0x0003d3, 0x0003d5, 0x0003d6, 0x0003f0, 0x0003f2, 0x0003f4, 0x0003f7, 0x000606, 0x000609, 0x002016, 0x002017, 0x002032, 0x002035, 
	0x002040, 0x002041, 0x002044, 0x002045, 0x002052, 0x002053, 0x002061, 0x002065, 0x00207a, 0x00207f, 0x00208a, 0x00208f, 0x0020d0, 0x0020dd, 0x0020e1, 0x0020e2, 
	0x0020e5, 0x0020e7, 0x0020eb, 0x0020f0, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 0x002118, 0x00211e, 0x002124, 0x002125, 
	0x002128, 0x00212a, 0x00212c, 0x00212e, 0x00212f, 0x002132, 0x002133, 0x002139, 0x00213c, 0x00214a, 0x00214b, 0x00214c, 0x002190, 0x0021a8, 0x0021a9, 0x0021af, 
	0x0021b0, 0x0021b2, 0x0021b6, 0x0021b8, 0x0021bc, 0x0021dc, 0x0021dd, 0x0021de, 0x0021e4, 0x0021e6, 0x0021f4, 0x002300, 0x002308, 0x00230c, 0x002320, 0x002322, 
	0x00237c, 0x00237d, 0x00239b, 0x0023b6, 0x0023b7, 0x0023b8, 0x0023d0, 0x0023d1, 0x0023dc, 0x0023e3, 0x0025a0, 0x0025a2, 0x0025ae, 0x0025b8, 0x0025bc, 0x0025c2, 
	0x0025c6, 0x0025c8, 0x0025ca, 0x0025cc, 0x0025cf, 0x0025d4, 0x0025e2, 0x0025e3, 0x0025e4, 0x0025e5, 0x0025e7, 0x0025ed, 0x0025f8, 0x002600, 0x002605, 0x002607, 
	0x002640, 0x002641, 0x002642, 0x002643, 0x002660, 0x002664, 0x00266d, 0x002670, 0x0027c0, 0x002800, 0x002900, 0x002b00, 0x002b30, 0x002b45, 0x002b47, 0x002b4d, 
	0x00fb29, 0x00fb2a, 0x00fe61, 0x00fe67, 0x00fe68, 0x00fe69, 0x00ff0b, 0x00ff0c, 0x00ff1c, 0x00ff1f, 0x00ff3c, 0x00ff3d, 0x00ff3e, 0x00ff3f, 0x00ff5c, 0x00ff5d, 
	0x00ff5e, 0x00ff5f, 0x00ffe2, 0x00ffe3, 0x00ffe9, 0x00ffed, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 
	0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 
	0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01d800, 
	0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 
	0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 
	0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 
	0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 
	0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 
};
#define mxCharSet_Binary_Property_Noncharacter_Code_Point 36
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Noncharacter_Code_Point[mxCharSet_Binary_Property_Noncharacter_Code_Point] = {
	0x00fdd0, 0x00fdf0, 0x00fffe, 0x010000, 0x01fffe, 0x020000, 0x02fffe, 0x030000, 0x03fffe, 0x040000, 0x04fffe, 0x050000, 0x05fffe, 0x060000, 0x06fffe, 0x070000, 
	0x07fffe, 0x080000, 0x08fffe, 0x090000, 0x09fffe, 0x0a0000, 0x0afffe, 0x0b0000, 0x0bfffe, 0x0c0000, 0x0cfffe, 0x0d0000, 0x0dfffe, 0x0e0000, 0x0efffe, 0x0f0000, 
	0x0ffffe, 0x100000, 0x10fffe, 0x110000, 
};
#define mxCharSet_Binary_Property_Pattern_Syntax 56
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Pattern_Syntax[mxCharSet_Binary_Property_Pattern_Syntax] = {
	0x000021, 0x000030, 0x00003a, 0x000041, 0x00005b, 0x00005f, 0x000060, 0x000061, 0x00007b, 0x00007f, 0x0000a1, 0x0000a8, 0x0000a9, 0x0000aa, 0x0000ab, 0x0000ad, 
	0x0000ae, 0x0000af, 0x0000b0, 0x0000b2, 0x0000b6, 0x0000b7, 0x0000bb, 0x0000bc, 0x0000bf, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x002010, 0x002028, 
	0x002030, 0x00203f, 0x002041, 0x002054, 0x002055, 0x00205f, 0x002190, 0x002460, 0x002500, 0x002776, 0x002794, 0x002c00, 0x002e00, 0x002e80, 0x003001, 0x003004, 
	0x003008, 0x003021, 0x003030, 0x003031, 0x00fd3e, 0x00fd40, 0x00fe45, 0x00fe47, 
};
#define mxCharSet_Binary_Property_Pattern_White_Space 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Pattern_White_Space[mxCharSet_Binary_Property_Pattern_White_Space] = {
	0x000009, 0x00000e, 0x000020, 0x000021, 0x000085, 0x000086, 0x00200e, 0x002010, 0x002028, 0x00202a, 
};
#define mxCharSet_Binary_Property_Quotation_Mark 26
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Quotation_Mark[mxCharSet_Binary_Property_Quotation_Mark] = {
	0x000022, 0x000023, 0x000027, 0x000028, 0x0000ab, 0x0000ac, 0x0000bb, 0x0000bc, 0x002018, 0x002020, 0x002039, 0x00203b, 0x002e42, 0x002e43, 0x00300c, 0x003010, 
	0x00301d, 0x003020, 0x00fe41, 0x00fe45, 0x00ff02, 0x00ff03, 0x00ff07, 0x00ff08, 0x00ff62, 0x00ff64, 
};
#define mxCharSet_Binary_Property_Radical 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Radical[mxCharSet_Binary_Property_Radical] = {
	0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 
};
#define mxCharSet_Binary_Property_Regional_Indicator 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Regional_Indicator[mxCharSet_Binary_Property_Regional_Indicator] = {
	0x01f1e6, 0x01f200, 
};
#define mxCharSet_Binary_Property_Sentence_Terminal 160
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Sentence_Terminal[mxCharSet_Binary_Property_Sentence_Terminal] = {
	0x000021, 0x000022, 0x00002e, 0x00002f, 0x00003f, 0x000040, 0x000589, 0x00058a, 0x00061d, 0x000620, 0x0006d4, 0x0006d5, 0x000700, 0x000703, 0x0007f9, 0x0007fa, 
	0x000837, 0x000838, 0x000839, 0x00083a, 0x00083d, 0x00083f, 0x000964, 0x000966, 0x00104a, 0x00104c, 0x001362, 0x001363, 0x001367, 0x001369, 0x00166e, 0x00166f, 
	0x001735, 0x001737, 0x001803, 0x001804, 0x001809, 0x00180a, 0x001944, 0x001946, 0x001aa8, 0x001aac, 0x001b5a, 0x001b5c, 0x001b5e, 0x001b60, 0x001b7d, 0x001b7f, 
	0x001c3b, 0x001c3d, 0x001c7e, 0x001c80, 0x00203c, 0x00203e, 0x002047, 0x00204a, 0x002e2e, 0x002e2f, 0x002e3c, 0x002e3d, 0x002e53, 0x002e55, 0x003002, 0x003003, 
	0x00a4ff, 0x00a500, 0x00a60e, 0x00a610, 0x00a6f3, 0x00a6f4, 0x00a6f7, 0x00a6f8, 0x00a876, 0x00a878, 0x00a8ce, 0x00a8d0, 0x00a92f, 0x00a930, 0x00a9c8, 0x00a9ca, 
	0x00aa5d, 0x00aa60, 0x00aaf0, 0x00aaf2, 0x00abeb, 0x00abec, 0x00fe52, 0x00fe53, 0x00fe56, 0x00fe58, 0x00ff01, 0x00ff02, 0x00ff0e, 0x00ff0f, 0x00ff1f, 0x00ff20, 
	0x00ff61, 0x00ff62, 0x010a56, 0x010a58, 0x010f55, 0x010f5a, 0x010f86, 0x010f8a, 0x011047, 0x011049, 0x0110be, 0x0110c2, 0x011141, 0x011144, 0x0111c5, 0x0111c7, 
	0x0111cd, 0x0111ce, 0x0111de, 0x0111e0, 0x011238, 0x01123a, 0x01123b, 0x01123d, 0x0112a9, 0x0112aa, 0x01144b, 0x01144d, 0x0115c2, 0x0115c4, 0x0115c9, 0x0115d8, 
	0x011641, 0x011643, 0x01173c, 0x01173f, 0x011944, 0x011945, 0x011946, 0x011947, 0x011a42, 0x011a44, 0x011a9b, 0x011a9d, 0x011c41, 0x011c43, 0x011ef7, 0x011ef9, 
	0x011f43, 0x011f45, 0x016a6e, 0x016a70, 0x016af5, 0x016af6, 0x016b37, 0x016b39, 0x016b44, 0x016b45, 0x016e98, 0x016e99, 0x01bc9f, 0x01bca0, 0x01da88, 0x01da89, 
};
#define mxCharSet_Binary_Property_Soft_Dotted 68
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Soft_Dotted[mxCharSet_Binary_Property_Soft_Dotted] = {
	0x000069, 0x00006b, 0x00012f, 0x000130, 0x000249, 0x00024a, 0x000268, 0x000269, 0x00029d, 0x00029e, 0x0002b2, 0x0002b3, 0x0003f3, 0x0003f4, 0x000456, 0x000457, 
	0x000458, 0x000459, 0x001d62, 0x001d63, 0x001d96, 0x001d97, 0x001da4, 0x001da5, 0x001da8, 0x001da9, 0x001e2d, 0x001e2e, 0x001ecb, 0x001ecc, 0x002071, 0x002072, 
	0x002148, 0x00214a, 0x002c7c, 0x002c7d, 0x01d422, 0x01d424, 0x01d456, 0x01d458, 0x01d48a, 0x01d48c, 0x01d4be, 0x01d4c0, 0x01d4f2, 0x01d4f4, 0x01d526, 0x01d528, 
	0x01d55a, 0x01d55c, 0x01d58e, 0x01d590, 0x01d5c2, 0x01d5c4, 0x01d5f6, 0x01d5f8, 0x01d62a, 0x01d62c, 0x01d65e, 0x01d660, 0x01d692, 0x01d694, 0x01df1a, 0x01df1b, 
	0x01e04c, 0x01e04e, 0x01e068, 0x01e069, 
};
#define mxCharSet_Binary_Property_Terminal_Punctuation 216
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Terminal_Punctuation[mxCharSet_Binary_Property_Terminal_Punctuation] = {
	0x000021, 0x000022, 0x00002c, 0x00002d, 0x00002e, 0x00002f, 0x00003a, 0x00003c, 0x00003f, 0x000040, 0x00037e, 0x00037f, 0x000387, 0x000388, 0x000589, 0x00058a, 
	0x0005c3, 0x0005c4, 0x00060c, 0x00060d, 0x00061b, 0x00061c, 0x00061d, 0x000620, 0x0006d4, 0x0006d5, 0x000700, 0x00070b, 0x00070c, 0x00070d, 0x0007f8, 0x0007fa, 
	0x000830, 0x00083f, 0x00085e, 0x00085f, 0x000964, 0x000966, 0x000e5a, 0x000e5c, 0x000f08, 0x000f09, 0x000f0d, 0x000f13, 0x00104a, 0x00104c, 0x001361, 0x001369, 
	0x00166e, 0x00166f, 0x0016eb, 0x0016ee, 0x001735, 0x001737, 0x0017d4, 0x0017d7, 0x0017da, 0x0017db, 0x001802, 0x001806, 0x001808, 0x00180a, 0x001944, 0x001946, 
	0x001aa8, 0x001aac, 0x001b5a, 0x001b5c, 0x001b5d, 0x001b60, 0x001b7d, 0x001b7f, 0x001c3b, 0x001c40, 0x001c7e, 0x001c80, 0x00203c, 0x00203e, 0x002047, 0x00204a, 
	0x002e2e, 0x002e2f, 0x002e3c, 0x002e3d, 0x002e41, 0x002e42, 0x002e4c, 0x002e4d, 0x002e4e, 0x002e50, 0x002e53, 0x002e55, 0x003001, 0x003003, 0x00a4fe, 0x00a500, 
	0x00a60d, 0x00a610, 0x00a6f3, 0x00a6f8, 0x00a876, 0x00a878, 0x00a8ce, 0x00a8d0, 0x00a92f, 0x00a930, 0x00a9c7, 0x00a9ca, 0x00aa5d, 0x00aa60, 0x00aadf, 0x00aae0, 
	0x00aaf0, 0x00aaf2, 0x00abeb, 0x00abec, 0x00fe50, 0x00fe53, 0x00fe54, 0x00fe58, 0x00ff01, 0x00ff02, 0x00ff0c, 0x00ff0d, 0x00ff0e, 0x00ff0f, 0x00ff1a, 0x00ff1c, 
	0x00ff1f, 0x00ff20, 0x00ff61, 0x00ff62, 0x00ff64, 0x00ff65, 0x01039f, 0x0103a0, 0x0103d0, 0x0103d1, 0x010857, 0x010858, 0x01091f, 0x010920, 0x010a56, 0x010a58, 
	0x010af0, 0x010af6, 0x010b3a, 0x010b40, 0x010b99, 0x010b9d, 0x010f55, 0x010f5a, 0x010f86, 0x010f8a, 0x011047, 0x01104e, 0x0110be, 0x0110c2, 0x011141, 0x011144, 
	0x0111c5, 0x0111c7, 0x0111cd, 0x0111ce, 0x0111de, 0x0111e0, 0x011238, 0x01123d, 0x0112a9, 0x0112aa, 0x01144b, 0x01144e, 0x01145a, 0x01145c, 0x0115c2, 0x0115c6, 
	0x0115c9, 0x0115d8, 0x011641, 0x011643, 0x01173c, 0x01173f, 0x011944, 0x011945, 0x011946, 0x011947, 0x011a42, 0x011a44, 0x011a9b, 0x011a9d, 0x011aa1, 0x011aa3, 
	0x011c41, 0x011c44, 0x011c71, 0x011c72, 0x011ef7, 0x011ef9, 0x011f43, 0x011f45, 0x012470, 0x012475, 0x016a6e, 0x016a70, 0x016af5, 0x016af6, 0x016b37, 0x016b3a, 
	0x016b44, 0x016b45, 0x016e97, 0x016e99, 0x01bc9f, 0x01bca0, 0x01da87, 0x01da8b, 
};
#define mxCharSet_Binary_Property_Unified_Ideograph 32
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Unified_Ideograph[mxCharSet_Binary_Property_Unified_Ideograph] = {
	0x003400, 0x004dc0, 0x004e00, 0x00a000, 0x00fa0e, 0x00fa10, 0x00fa11, 0x00fa12, 0x00fa13, 0x00fa15, 0x00fa1f, 0x00fa20, 0x00fa21, 0x00fa22, 0x00fa23, 0x00fa25, 
	0x00fa27, 0x00fa2a, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Binary_Property_Uppercase 1302
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Uppercase[mxCharSet_Binary_Property_Uppercase] = {
	0x000041, 0x00005b, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000df, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 0x000109, 
	0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 0x000119, 
	0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 0x000129, 
	0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000139, 0x00013a, 
	0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 0x00014b, 
	0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 0x00015b, 
	0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 0x00016b, 
	0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 0x00017c, 
	0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 0x000193, 0x000195, 0x000196, 0x000199, 
	0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 
	0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bc, 0x0001bd, 0x0001c4, 0x0001c5, 0x0001c7, 0x0001c8, 0x0001ca, 0x0001cb, 0x0001cd, 0x0001ce, 
	0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 0x0001df, 
	0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 
	0x0001f1, 0x0001f2, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 0x000203, 
	0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 0x000213, 
	0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 0x000221, 0x000222, 0x000223, 
	0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 
	0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 
	0x000370, 0x000371, 0x000372, 0x000373, 0x000376, 0x000377, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x000390, 
	0x000391, 0x0003a2, 0x0003a3, 0x0003ac, 0x0003cf, 0x0003d0, 0x0003d2, 0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 
	0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 
	0x0003f4, 0x0003f5, 0x0003f7, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 
	0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 
	0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x00048a, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 
	0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 
	0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 
	0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 
	0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 
	0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 
	0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 
	0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 
	0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 
	0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 
	0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000531, 0x000557, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0013a0, 0x0013f6, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001e00, 0x001e01, 0x001e02, 0x001e03, 
	0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 
	0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 
	0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 
	0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 
	0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 
	0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 
	0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 
	0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 
	0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 
	0x001e94, 0x001e95, 0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 
	0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 
	0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 
	0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 
	0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 
	0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 
	0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f18, 0x001f1e, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f48, 0x001f4e, 0x001f59, 0x001f5a, 
	0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f60, 0x001f68, 0x001f70, 0x001fb8, 0x001fbc, 0x001fc8, 0x001fcc, 0x001fd8, 0x001fdc, 0x001fe8, 0x001fed, 
	0x001ff8, 0x001ffc, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210b, 0x00210e, 0x002110, 0x002113, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 
	0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x002130, 0x002134, 0x00213e, 0x002140, 0x002145, 0x002146, 0x002160, 0x002170, 0x002183, 0x002184, 
	0x0024b6, 0x0024d0, 0x002c00, 0x002c30, 0x002c60, 0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 
	0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7e, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 
	0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 
	0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 
	0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 
	0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 
	0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 
	0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ceb, 0x002cec, 0x002ced, 0x002cee, 0x002cf2, 0x002cf3, 0x00a640, 0x00a641, 
	0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 
	0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 
	0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a680, 0x00a681, 0x00a682, 0x00a683, 
	0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 
	0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a722, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 0x00a729, 
	0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 
	0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 
	0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 
	0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 
	0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 0x00a785, 
	0x00a786, 0x00a787, 0x00a78b, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a790, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 
	0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7af, 
	0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 
	0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7d0, 0x00a7d1, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7f5, 0x00a7f6, 0x00ff21, 0x00ff3b, 0x010400, 0x010428, 
	0x0104b0, 0x0104d4, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010c80, 0x010cb3, 0x0118a0, 0x0118c0, 0x016e40, 0x016e60, 
	0x01d400, 0x01d41a, 0x01d434, 0x01d44e, 0x01d468, 0x01d482, 0x01d49c, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 
	0x01d4ae, 0x01d4b6, 0x01d4d0, 0x01d4ea, 0x01d504, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d538, 0x01d53a, 0x01d53b, 0x01d53f, 
	0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d56c, 0x01d586, 0x01d5a0, 0x01d5ba, 0x01d5d4, 0x01d5ee, 0x01d608, 0x01d622, 0x01d63c, 0x01d656, 
	0x01d670, 0x01d68a, 0x01d6a8, 0x01d6c1, 0x01d6e2, 0x01d6fb, 0x01d71c, 0x01d735, 0x01d756, 0x01d76f, 0x01d790, 0x01d7a9, 0x01d7ca, 0x01d7cb, 0x01e900, 0x01e922, 
	0x01f130, 0x01f14a, 0x01f150, 0x01f16a, 0x01f170, 0x01f18a, 
};
#define mxCharSet_Binary_Property_Variation_Selector 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_Variation_Selector[mxCharSet_Binary_Property_Variation_Selector] = {
	0x00180b, 0x00180e, 0x00180f, 0x001810, 0x00fe00, 0x00fe10, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Binary_Property_White_Space 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_White_Space[mxCharSet_Binary_Property_White_Space] = {
	0x000009, 0x00000e, 0x000020, 0x000021, 0x000085, 0x000086, 0x0000a0, 0x0000a1, 0x001680, 0x001681, 0x002000, 0x00200b, 0x002028, 0x00202a, 0x00202f, 0x002030, 
	0x00205f, 0x002060, 0x003000, 0x003001, 
};
#define mxCharSet_Binary_Property_XID_Continue 1550
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_XID_Continue[mxCharSet_Binary_Property_XID_Continue] = {
	0x000030, 0x00003a, 0x000041, 0x00005b, 0x00005f, 0x000060, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000b7, 0x0000b8, 0x0000ba, 0x0000bb, 
	0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000300, 0x000375, 
	0x000376, 0x000378, 0x00037b, 0x00037e, 0x00037f, 0x000380, 0x000386, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 
	0x000483, 0x000488, 0x00048a, 0x000530, 0x000531, 0x000557, 0x000559, 0x00055a, 0x000560, 0x000589, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 
	0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000610, 0x00061b, 0x000620, 0x00066a, 0x00066e, 0x0006d4, 0x0006d5, 0x0006dd, 
	0x0006df, 0x0006e9, 0x0006ea, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x00074b, 0x00074d, 0x0007b2, 0x0007c0, 0x0007f6, 0x0007fa, 0x0007fb, 0x0007fd, 0x0007fe, 
	0x000800, 0x00082e, 0x000840, 0x00085c, 0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x000898, 0x0008e2, 0x0008e3, 0x000964, 0x000966, 0x000970, 
	0x000971, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 
	0x0009c7, 0x0009c9, 0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009f2, 0x0009fc, 0x0009fd, 0x0009fe, 0x0009ff, 
	0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 
	0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a76, 
	0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 
	0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 0x000ae6, 0x000af0, 0x000af9, 0x000b00, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 
	0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 
	0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 0x000b70, 0x000b71, 0x000b72, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 
	0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 
	0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000be6, 0x000bf0, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 
	0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 
	0x000c66, 0x000c70, 0x000c80, 0x000c84, 0x000c85, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 
	0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 
	0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d4f, 0x000d54, 0x000d58, 0x000d5f, 0x000d64, 0x000d66, 0x000d70, 0x000d7a, 0x000d80, 
	0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd5, 
	0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df4, 0x000e01, 0x000e3b, 0x000e40, 0x000e4f, 0x000e50, 0x000e5a, 0x000e81, 0x000e83, 
	0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 
	0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f01, 0x000f18, 0x000f1a, 0x000f20, 0x000f2a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 
	0x000f3e, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f85, 0x000f86, 0x000f98, 0x000f99, 0x000fbd, 0x000fc6, 0x000fc7, 0x001000, 0x00104a, 0x001050, 0x00109e, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 
	0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 
	0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x00135d, 0x001360, 0x001369, 0x001372, 0x001380, 0x001390, 0x0013a0, 0x0013f6, 
	0x0013f8, 0x0013fe, 0x001401, 0x00166d, 0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 0x001700, 0x001716, 0x00171f, 0x001735, 
	0x001740, 0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017d4, 0x0017d7, 0x0017d8, 0x0017dc, 0x0017de, 0x0017e0, 0x0017ea, 
	0x00180b, 0x00180e, 0x00180f, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x00193c, 
	0x001946, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x001a00, 0x001a1c, 0x001a20, 0x001a5f, 0x001a60, 0x001a7d, 
	0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa7, 0x001aa8, 0x001ab0, 0x001abe, 0x001abf, 0x001acf, 0x001b00, 0x001b4d, 0x001b50, 0x001b5a, 0x001b6b, 0x001b74, 
	0x001b80, 0x001bf4, 0x001c00, 0x001c38, 0x001c40, 0x001c4a, 0x001c4d, 0x001c7e, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001cd0, 0x001cd3, 
	0x001cd4, 0x001cfb, 0x001d00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 
	0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 
	0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x00203f, 0x002041, 0x002054, 0x002055, 0x002071, 0x002072, 0x00207f, 0x002080, 
	0x002090, 0x00209d, 0x0020d0, 0x0020dd, 0x0020e1, 0x0020e2, 0x0020e5, 0x0020f1, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 
	0x002118, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 
	0x002160, 0x002189, 0x002c00, 0x002ce5, 0x002ceb, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d70, 
	0x002d7f, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 
	0x002dd8, 0x002ddf, 0x002de0, 0x002e00, 0x003005, 0x003008, 0x003021, 0x003030, 0x003031, 0x003036, 0x003038, 0x00303d, 0x003041, 0x003097, 0x003099, 0x00309b, 
	0x00309d, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 0x003400, 0x004dc0, 
	0x004e00, 0x00a48d, 0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a62c, 0x00a640, 0x00a670, 0x00a674, 0x00a67e, 0x00a67f, 0x00a6f2, 0x00a717, 0x00a720, 
	0x00a722, 0x00a789, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a828, 0x00a82c, 0x00a82d, 0x00a840, 0x00a874, 
	0x00a880, 0x00a8c6, 0x00a8d0, 0x00a8da, 0x00a8e0, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a92e, 0x00a930, 0x00a954, 0x00a960, 0x00a97d, 0x00a980, 0x00a9c1, 
	0x00a9cf, 0x00a9da, 0x00a9e0, 0x00a9ff, 0x00aa00, 0x00aa37, 0x00aa40, 0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aac3, 0x00aadb, 0x00aade, 
	0x00aae0, 0x00aaf0, 0x00aaf2, 0x00aaf7, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 
	0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abeb, 0x00abec, 0x00abee, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 
	0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb29, 0x00fb2a, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 
	0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fc5e, 0x00fc64, 0x00fd3e, 0x00fd50, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfa, 0x00fe00, 0x00fe10, 
	0x00fe20, 0x00fe30, 0x00fe33, 0x00fe35, 0x00fe4d, 0x00fe50, 0x00fe71, 0x00fe72, 0x00fe73, 0x00fe74, 0x00fe77, 0x00fe78, 0x00fe79, 0x00fe7a, 0x00fe7b, 0x00fe7c, 
	0x00fe7d, 0x00fe7e, 0x00fe7f, 0x00fefd, 0x00ff10, 0x00ff1a, 0x00ff21, 0x00ff3b, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ffbf, 0x00ffc2, 0x00ffc8, 
	0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 
	0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010140, 0x010175, 0x0101fd, 0x0101fe, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x0102e0, 0x0102e1, 0x010300, 0x010320, 
	0x01032d, 0x01034b, 0x010350, 0x01037b, 0x010380, 0x01039e, 0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x0103d1, 0x0103d6, 0x010400, 0x01049e, 0x0104a0, 0x0104aa, 
	0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 
	0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 
	0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 
	0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 
	0x010a00, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 0x010a60, 0x010a7d, 
	0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae7, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 0x010b60, 0x010b73, 0x010b80, 0x010b92, 0x010c00, 0x010c49, 
	0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d28, 0x010d30, 0x010d3a, 0x010e80, 0x010eaa, 0x010eab, 0x010ead, 0x010eb0, 0x010eb2, 0x010efd, 0x010f1d, 
	0x010f27, 0x010f28, 0x010f30, 0x010f51, 0x010f70, 0x010f86, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011000, 0x011047, 0x011066, 0x011076, 0x01107f, 0x0110bb, 
	0x0110c2, 0x0110c3, 0x0110d0, 0x0110e9, 0x0110f0, 0x0110fa, 0x011100, 0x011135, 0x011136, 0x011140, 0x011144, 0x011148, 0x011150, 0x011174, 0x011176, 0x011177, 
	0x011180, 0x0111c5, 0x0111c9, 0x0111cd, 0x0111ce, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x011238, 0x01123e, 0x011242, 0x011280, 0x011287, 
	0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 0x011300, 0x011304, 0x011305, 0x01130d, 
	0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133b, 0x011345, 0x011347, 0x011349, 0x01134b, 0x01134e, 
	0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011400, 0x01144b, 0x011450, 0x01145a, 0x01145e, 0x011462, 
	0x011480, 0x0114c6, 0x0114c7, 0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115b6, 0x0115b8, 0x0115c1, 0x0115d8, 0x0115de, 0x011600, 0x011641, 0x011644, 0x011645, 
	0x011650, 0x01165a, 0x011680, 0x0116b9, 0x0116c0, 0x0116ca, 0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x01173a, 0x011740, 0x011747, 0x011800, 0x01183b, 
	0x0118a0, 0x0118ea, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011944, 
	0x011950, 0x01195a, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 0x0119e2, 0x0119e3, 0x0119e5, 0x011a00, 0x011a3f, 0x011a47, 0x011a48, 0x011a50, 0x011a9a, 
	0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c41, 0x011c50, 0x011c5a, 0x011c72, 0x011c90, 0x011c92, 0x011ca8, 
	0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 
	0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 0x011daa, 0x011ee0, 0x011ef7, 0x011f00, 0x011f11, 
	0x011f12, 0x011f3b, 0x011f3e, 0x011f43, 0x011f50, 0x011f5a, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 0x012400, 0x01246f, 0x012480, 0x012544, 0x012f90, 0x012ff1, 
	0x013000, 0x013430, 0x013440, 0x013456, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a70, 0x016abf, 0x016ac0, 0x016aca, 
	0x016ad0, 0x016aee, 0x016af0, 0x016af5, 0x016b00, 0x016b37, 0x016b40, 0x016b44, 0x016b50, 0x016b5a, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e80, 
	0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe5, 0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 
	0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 
	0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9d, 0x01bc9f, 0x01cf00, 0x01cf2e, 
	0x01cf30, 0x01cf47, 0x01d165, 0x01d16a, 0x01d16d, 0x01d173, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 0x01d400, 0x01d455, 
	0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 
	0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 
	0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 
	0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01d7ce, 0x01d800, 0x01da00, 0x01da37, 0x01da3b, 0x01da6d, 
	0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 
	0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 0x01e140, 0x01e14a, 
	0x01e14e, 0x01e14f, 0x01e290, 0x01e2af, 0x01e2c0, 0x01e2fa, 0x01e4d0, 0x01e4fa, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 
	0x01e800, 0x01e8c5, 0x01e8d0, 0x01e8d7, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 
	0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 
	0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 
	0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 
	0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01fbf0, 0x01fbfa, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 
	0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Binary_Property_XID_Start 1332
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Binary_Property_XID_Start[mxCharSet_Binary_Property_XID_Start] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 
	0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000370, 0x000375, 0x000376, 0x000378, 0x00037b, 0x00037e, 0x00037f, 0x000380, 
	0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 0x00048a, 0x000530, 0x000531, 0x000557, 
	0x000559, 0x00055a, 0x000560, 0x000589, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000620, 0x00064b, 0x00066e, 0x000670, 0x000671, 0x0006d4, 0x0006d5, 0x0006d6, 
	0x0006e5, 0x0006e7, 0x0006ee, 0x0006f0, 0x0006fa, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x000711, 0x000712, 0x000730, 0x00074d, 0x0007a6, 0x0007b1, 0x0007b2, 
	0x0007ca, 0x0007eb, 0x0007f4, 0x0007f6, 0x0007fa, 0x0007fb, 0x000800, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x000840, 0x000859, 
	0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x0008a0, 0x0008ca, 0x000904, 0x00093a, 0x00093d, 0x00093e, 0x000950, 0x000951, 0x000958, 0x000962, 
	0x000971, 0x000981, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009be, 
	0x0009ce, 0x0009cf, 0x0009dc, 0x0009de, 0x0009df, 0x0009e2, 0x0009f0, 0x0009f2, 0x0009fc, 0x0009fd, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 
	0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a72, 0x000a75, 0x000a85, 0x000a8e, 
	0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000abe, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae2, 
	0x000af9, 0x000afa, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3d, 0x000b3e, 
	0x000b5c, 0x000b5e, 0x000b5f, 0x000b62, 0x000b71, 0x000b72, 0x000b83, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 
	0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bd0, 0x000bd1, 0x000c05, 0x000c0d, 0x000c0e, 0x000c11, 
	0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3d, 0x000c3e, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c62, 0x000c80, 0x000c81, 0x000c85, 0x000c8d, 
	0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cbe, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce2, 0x000cf1, 0x000cf3, 
	0x000d04, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d4e, 0x000d4f, 0x000d54, 0x000d57, 0x000d5f, 0x000d62, 0x000d7a, 0x000d80, 
	0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000e01, 0x000e31, 0x000e32, 0x000e33, 0x000e40, 0x000e47, 
	0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eb1, 0x000eb2, 0x000eb3, 0x000ebd, 0x000ebe, 
	0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000edc, 0x000ee0, 0x000f00, 0x000f01, 0x000f40, 0x000f48, 0x000f49, 0x000f6d, 0x000f88, 0x000f8d, 0x001000, 0x00102b, 
	0x00103f, 0x001040, 0x001050, 0x001056, 0x00105a, 0x00105e, 0x001061, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001071, 0x001075, 0x001082, 0x00108e, 0x00108f, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 
	0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 
	0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001380, 0x001390, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001401, 0x00166d, 
	0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 0x001700, 0x001712, 0x00171f, 0x001732, 0x001740, 0x001752, 0x001760, 0x00176d, 
	0x00176e, 0x001771, 0x001780, 0x0017b4, 0x0017d7, 0x0017d8, 0x0017dc, 0x0017dd, 0x001820, 0x001879, 0x001880, 0x0018a9, 0x0018aa, 0x0018ab, 0x0018b0, 0x0018f6, 
	0x001900, 0x00191f, 0x001950, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x001a00, 0x001a17, 0x001a20, 0x001a55, 0x001aa7, 0x001aa8, 
	0x001b05, 0x001b34, 0x001b45, 0x001b4d, 0x001b83, 0x001ba1, 0x001bae, 0x001bb0, 0x001bba, 0x001be6, 0x001c00, 0x001c24, 0x001c4d, 0x001c50, 0x001c5a, 0x001c7e, 
	0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001cfb, 0x001d00, 0x001dc0, 
	0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 
	0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 
	0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x002102, 0x002103, 0x002107, 0x002108, 
	0x00210a, 0x002114, 0x002115, 0x002116, 0x002118, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00213a, 0x00213c, 0x002140, 
	0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002160, 0x002189, 0x002c00, 0x002ce5, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 
		0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d70, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 
	0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x003005, 0x003008, 0x003021, 0x00302a, 0x003031, 0x003036, 0x003038, 0x00303d, 
	0x003041, 0x003097, 0x00309d, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 
	0x003400, 0x004dc0, 0x004e00, 0x00a48d, 0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a620, 0x00a62a, 0x00a62c, 0x00a640, 0x00a66f, 0x00a67f, 0x00a69e, 
	0x00a6a0, 0x00a6f0, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a802, 
	0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a823, 0x00a840, 0x00a874, 0x00a882, 0x00a8b4, 0x00a8f2, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a8ff, 
	0x00a90a, 0x00a926, 0x00a930, 0x00a947, 0x00a960, 0x00a97d, 0x00a984, 0x00a9b3, 0x00a9cf, 0x00a9d0, 0x00a9e0, 0x00a9e5, 0x00a9e6, 0x00a9f0, 0x00a9fa, 0x00a9ff, 
	0x00aa00, 0x00aa29, 0x00aa40, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aa7b, 0x00aa7e, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 
	0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 0x00aadb, 0x00aade, 0x00aae0, 0x00aaeb, 0x00aaf2, 0x00aaf5, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 
	0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abe3, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 
	0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb1e, 0x00fb1f, 0x00fb29, 0x00fb2a, 0x00fb37, 
	0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fc5e, 0x00fc64, 0x00fd3e, 0x00fd50, 0x00fd90, 
	0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfa, 0x00fe71, 0x00fe72, 0x00fe73, 0x00fe74, 0x00fe77, 0x00fe78, 0x00fe79, 0x00fe7a, 0x00fe7b, 0x00fe7c, 0x00fe7d, 0x00fe7e, 
	0x00fe7f, 0x00fefd, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ff9e, 0x00ffa0, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 
	0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 
	0x010140, 0x010175, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x010300, 0x010320, 0x01032d, 0x01034b, 0x010350, 0x010376, 0x010380, 0x01039e, 0x0103a0, 0x0103c4, 
	0x0103c8, 0x0103d0, 0x0103d1, 0x0103d6, 0x010400, 0x01049e, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 0x010570, 0x01057b, 
	0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010600, 0x010737, 
	0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 
	0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x010900, 0x010916, 
	0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 0x010a00, 0x010a01, 0x010a10, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a60, 0x010a7d, 
	0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae5, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 0x010b60, 0x010b73, 0x010b80, 0x010b92, 0x010c00, 0x010c49, 
	0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d24, 0x010e80, 0x010eaa, 0x010eb0, 0x010eb2, 0x010f00, 0x010f1d, 0x010f27, 0x010f28, 0x010f30, 0x010f46, 
	0x010f70, 0x010f82, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011003, 0x011038, 0x011071, 0x011073, 0x011075, 0x011076, 0x011083, 0x0110b0, 0x0110d0, 0x0110e9, 
	0x011103, 0x011127, 0x011144, 0x011145, 0x011147, 0x011148, 0x011150, 0x011173, 0x011176, 0x011177, 0x011183, 0x0111b3, 0x0111c1, 0x0111c5, 0x0111da, 0x0111db, 
	0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x01122c, 0x01123f, 0x011241, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 
	0x01129f, 0x0112a9, 0x0112b0, 0x0112df, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 
	0x01133d, 0x01133e, 0x011350, 0x011351, 0x01135d, 0x011362, 0x011400, 0x011435, 0x011447, 0x01144b, 0x01145f, 0x011462, 0x011480, 0x0114b0, 0x0114c4, 0x0114c6, 
	0x0114c7, 0x0114c8, 0x011580, 0x0115af, 0x0115d8, 0x0115dc, 0x011600, 0x011630, 0x011644, 0x011645, 0x011680, 0x0116ab, 0x0116b8, 0x0116b9, 0x011700, 0x01171b, 
	0x011740, 0x011747, 0x011800, 0x01182c, 0x0118a0, 0x0118e0, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011930, 
	0x01193f, 0x011940, 0x011941, 0x011942, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d1, 0x0119e1, 0x0119e2, 0x0119e3, 0x0119e4, 0x011a00, 0x011a01, 0x011a0b, 0x011a33, 
	0x011a3a, 0x011a3b, 0x011a50, 0x011a51, 0x011a5c, 0x011a8a, 0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c2f, 0x011c40, 0x011c41, 
	0x011c72, 0x011c90, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d31, 0x011d46, 0x011d47, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8a, 
	0x011d98, 0x011d99, 0x011ee0, 0x011ef3, 0x011f02, 0x011f03, 0x011f04, 0x011f11, 0x011f12, 0x011f34, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 0x012400, 0x01246f, 
	0x012480, 0x012544, 0x012f90, 0x012ff1, 0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a70, 0x016abf, 
	0x016ad0, 0x016aee, 0x016b00, 0x016b30, 0x016b40, 0x016b44, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e80, 0x016f00, 0x016f4b, 0x016f50, 0x016f51, 
	0x016f93, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe4, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 
	0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 
	0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 
	0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 
	0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 
	0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 
	0x01d7c4, 0x01d7cc, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e030, 0x01e06e, 0x01e100, 0x01e12d, 0x01e137, 0x01e13e, 0x01e14e, 0x01e14f, 0x01e290, 0x01e2ae, 
	0x01e2c0, 0x01e2ec, 0x01e4d0, 0x01e4ec, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e900, 0x01e944, 
	0x01e94b, 0x01e94c, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 
	0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 
	0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 
	0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 
	0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 
	0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_General_Category_Cased_Letter 286
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Cased_Letter[mxCharSet_General_Category_Cased_Letter] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000b5, 0x0000b6, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0001bb, 0x0001bc, 0x0001c0, 0x0001c4, 0x000294, 
	0x000295, 0x0002b0, 0x000370, 0x000374, 0x000376, 0x000378, 0x00037b, 0x00037e, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 
	0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 0x00048a, 0x000530, 0x000531, 0x000557, 0x000560, 0x000589, 0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 
	0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fd, 0x001100, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 
	0x001d00, 0x001d2c, 0x001d6b, 0x001d78, 0x001d79, 0x001d9b, 0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 
	0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 
	0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x002102, 0x002103, 0x002107, 0x002108, 
	0x00210a, 0x002114, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x002135, 
	0x002139, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002183, 0x002185, 0x002c00, 0x002c7c, 0x002c7e, 0x002ce5, 0x002ceb, 0x002cef, 
	0x002cf2, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x00a640, 0x00a66e, 0x00a680, 0x00a69c, 0x00a722, 0x00a770, 0x00a771, 0x00a788, 
	0x00a78b, 0x00a78f, 0x00a790, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f5, 0x00a7f7, 0x00a7fa, 0x00a7fb, 0x00ab30, 0x00ab5b, 
	0x00ab60, 0x00ab69, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x010400, 0x010450, 0x0104b0, 0x0104d4, 
	0x0104d8, 0x0104fc, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 
	0x0105bb, 0x0105bd, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x0118a0, 0x0118e0, 0x016e40, 0x016e80, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 
	0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 
	0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 
	0x01d6a8, 0x01d6c1, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 
	0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01df00, 0x01df0a, 0x01df0b, 0x01df1f, 0x01df25, 0x01df2b, 0x01e900, 0x01e944, 
};
#define mxCharSet_General_Category_Close_Punctuation 152
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Close_Punctuation[mxCharSet_General_Category_Close_Punctuation] = {
	0x000029, 0x00002a, 0x00005d, 0x00005e, 0x00007d, 0x00007e, 0x000f3b, 0x000f3c, 0x000f3d, 0x000f3e, 0x00169c, 0x00169d, 0x002046, 0x002047, 0x00207e, 0x00207f, 
	0x00208e, 0x00208f, 0x002309, 0x00230a, 0x00230b, 0x00230c, 0x00232a, 0x00232b, 0x002769, 0x00276a, 0x00276b, 0x00276c, 0x00276d, 0x00276e, 0x00276f, 0x002770, 
	0x002771, 0x002772, 0x002773, 0x002774, 0x002775, 0x002776, 0x0027c6, 0x0027c7, 0x0027e7, 0x0027e8, 0x0027e9, 0x0027ea, 0x0027eb, 0x0027ec, 0x0027ed, 0x0027ee, 
	0x0027ef, 0x0027f0, 0x002984, 0x002985, 0x002986, 0x002987, 0x002988, 0x002989, 0x00298a, 0x00298b, 0x00298c, 0x00298d, 0x00298e, 0x00298f, 0x002990, 0x002991, 
	0x002992, 0x002993, 0x002994, 0x002995, 0x002996, 0x002997, 0x002998, 0x002999, 0x0029d9, 0x0029da, 0x0029db, 0x0029dc, 0x0029fd, 0x0029fe, 0x002e23, 0x002e24, 
	0x002e25, 0x002e26, 0x002e27, 0x002e28, 0x002e29, 0x002e2a, 0x002e56, 0x002e57, 0x002e58, 0x002e59, 0x002e5a, 0x002e5b, 0x002e5c, 0x002e5d, 0x003009, 0x00300a, 
	0x00300b, 0x00300c, 0x00300d, 0x00300e, 0x00300f, 0x003010, 0x003011, 0x003012, 0x003015, 0x003016, 0x003017, 0x003018, 0x003019, 0x00301a, 0x00301b, 0x00301c, 
	0x00301e, 0x003020, 0x00fd3e, 0x00fd3f, 0x00fe18, 0x00fe19, 0x00fe36, 0x00fe37, 0x00fe38, 0x00fe39, 0x00fe3a, 0x00fe3b, 0x00fe3c, 0x00fe3d, 0x00fe3e, 0x00fe3f, 
	0x00fe40, 0x00fe41, 0x00fe42, 0x00fe43, 0x00fe44, 0x00fe45, 0x00fe48, 0x00fe49, 0x00fe5a, 0x00fe5b, 0x00fe5c, 0x00fe5d, 0x00fe5e, 0x00fe5f, 0x00ff09, 0x00ff0a, 
	0x00ff3d, 0x00ff3e, 0x00ff5d, 0x00ff5e, 0x00ff60, 0x00ff61, 0x00ff63, 0x00ff64, 
};
#define mxCharSet_General_Category_Connector_Punctuation 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Connector_Punctuation[mxCharSet_General_Category_Connector_Punctuation] = {
	0x00005f, 0x000060, 0x00203f, 0x002041, 0x002054, 0x002055, 0x00fe33, 0x00fe35, 0x00fe4d, 0x00fe50, 0x00ff3f, 0x00ff40, 
};
#define mxCharSet_General_Category_Control 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Control[mxCharSet_General_Category_Control] = {
	0x000000, 0x000020, 0x00007f, 0x0000a0, 
};
#define mxCharSet_General_Category_Currency_Symbol 42
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Currency_Symbol[mxCharSet_General_Category_Currency_Symbol] = {
	0x000024, 0x000025, 0x0000a2, 0x0000a6, 0x00058f, 0x000590, 0x00060b, 0x00060c, 0x0007fe, 0x000800, 0x0009f2, 0x0009f4, 0x0009fb, 0x0009fc, 0x000af1, 0x000af2, 
	0x000bf9, 0x000bfa, 0x000e3f, 0x000e40, 0x0017db, 0x0017dc, 0x0020a0, 0x0020c1, 0x00a838, 0x00a839, 0x00fdfc, 0x00fdfd, 0x00fe69, 0x00fe6a, 0x00ff04, 0x00ff05, 
	0x00ffe0, 0x00ffe2, 0x00ffe5, 0x00ffe7, 0x011fdd, 0x011fe1, 0x01e2ff, 0x01e300, 0x01ecb0, 0x01ecb1, 
};
#define mxCharSet_General_Category_Dash_Punctuation 38
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Dash_Punctuation[mxCharSet_General_Category_Dash_Punctuation] = {
	0x00002d, 0x00002e, 0x00058a, 0x00058b, 0x0005be, 0x0005bf, 0x001400, 0x001401, 0x001806, 0x001807, 0x002010, 0x002016, 0x002e17, 0x002e18, 0x002e1a, 0x002e1b, 
	0x002e3a, 0x002e3c, 0x002e40, 0x002e41, 0x002e5d, 0x002e5e, 0x00301c, 0x00301d, 0x003030, 0x003031, 0x0030a0, 0x0030a1, 0x00fe31, 0x00fe33, 0x00fe58, 0x00fe59, 
	0x00fe63, 0x00fe64, 0x00ff0d, 0x00ff0e, 0x010ead, 0x010eae, 
};
#define mxCharSet_General_Category_Decimal_Number 128
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Decimal_Number[mxCharSet_General_Category_Decimal_Number] = {
	0x000030, 0x00003a, 0x000660, 0x00066a, 0x0006f0, 0x0006fa, 0x0007c0, 0x0007ca, 0x000966, 0x000970, 0x0009e6, 0x0009f0, 0x000a66, 0x000a70, 0x000ae6, 0x000af0, 
	0x000b66, 0x000b70, 0x000be6, 0x000bf0, 0x000c66, 0x000c70, 0x000ce6, 0x000cf0, 0x000d66, 0x000d70, 0x000de6, 0x000df0, 0x000e50, 0x000e5a, 0x000ed0, 0x000eda, 
	0x000f20, 0x000f2a, 0x001040, 0x00104a, 0x001090, 0x00109a, 0x0017e0, 0x0017ea, 0x001810, 0x00181a, 0x001946, 0x001950, 0x0019d0, 0x0019da, 0x001a80, 0x001a8a, 
	0x001a90, 0x001a9a, 0x001b50, 0x001b5a, 0x001bb0, 0x001bba, 0x001c40, 0x001c4a, 0x001c50, 0x001c5a, 0x00a620, 0x00a62a, 0x00a8d0, 0x00a8da, 0x00a900, 0x00a90a, 
	0x00a9d0, 0x00a9da, 0x00a9f0, 0x00a9fa, 0x00aa50, 0x00aa5a, 0x00abf0, 0x00abfa, 0x00ff10, 0x00ff1a, 0x0104a0, 0x0104aa, 0x010d30, 0x010d3a, 0x011066, 0x011070, 
	0x0110f0, 0x0110fa, 0x011136, 0x011140, 0x0111d0, 0x0111da, 0x0112f0, 0x0112fa, 0x011450, 0x01145a, 0x0114d0, 0x0114da, 0x011650, 0x01165a, 0x0116c0, 0x0116ca, 
	0x011730, 0x01173a, 0x0118e0, 0x0118ea, 0x011950, 0x01195a, 0x011c50, 0x011c5a, 0x011d50, 0x011d5a, 0x011da0, 0x011daa, 0x011f50, 0x011f5a, 0x016a60, 0x016a6a, 
	0x016ac0, 0x016aca, 0x016b50, 0x016b5a, 0x01d7ce, 0x01d800, 0x01e140, 0x01e14a, 0x01e2f0, 0x01e2fa, 0x01e4f0, 0x01e4fa, 0x01e950, 0x01e95a, 0x01fbf0, 0x01fbfa, 
};
#define mxCharSet_General_Category_Enclosing_Mark 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Enclosing_Mark[mxCharSet_General_Category_Enclosing_Mark] = {
	0x000488, 0x00048a, 0x001abe, 0x001abf, 0x0020dd, 0x0020e1, 0x0020e2, 0x0020e5, 0x00a670, 0x00a673, 
};
#define mxCharSet_General_Category_Final_Punctuation 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Final_Punctuation[mxCharSet_General_Category_Final_Punctuation] = {
	0x0000bb, 0x0000bc, 0x002019, 0x00201a, 0x00201d, 0x00201e, 0x00203a, 0x00203b, 0x002e03, 0x002e04, 0x002e05, 0x002e06, 0x002e0a, 0x002e0b, 0x002e0d, 0x002e0e, 
	0x002e1d, 0x002e1e, 0x002e21, 0x002e22, 
};
#define mxCharSet_General_Category_Format 42
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Format[mxCharSet_General_Category_Format] = {
	0x0000ad, 0x0000ae, 0x000600, 0x000606, 0x00061c, 0x00061d, 0x0006dd, 0x0006de, 0x00070f, 0x000710, 0x000890, 0x000892, 0x0008e2, 0x0008e3, 0x00180e, 0x00180f, 
	0x00200b, 0x002010, 0x00202a, 0x00202f, 0x002060, 0x002065, 0x002066, 0x002070, 0x00feff, 0x00ff00, 0x00fff9, 0x00fffc, 0x0110bd, 0x0110be, 0x0110cd, 0x0110ce, 
	0x013430, 0x013440, 0x01bca0, 0x01bca4, 0x01d173, 0x01d17b, 0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 
};
#define mxCharSet_General_Category_Initial_Punctuation 22
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Initial_Punctuation[mxCharSet_General_Category_Initial_Punctuation] = {
	0x0000ab, 0x0000ac, 0x002018, 0x002019, 0x00201b, 0x00201d, 0x00201f, 0x002020, 0x002039, 0x00203a, 0x002e02, 0x002e03, 0x002e04, 0x002e05, 0x002e09, 0x002e0a, 
	0x002e0c, 0x002e0d, 0x002e1c, 0x002e1d, 0x002e20, 0x002e21, 
};
#define mxCharSet_General_Category_Letter 1318
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Letter[mxCharSet_General_Category_Letter] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000b5, 0x0000b6, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 
	0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000370, 0x000375, 0x000376, 0x000378, 0x00037a, 0x00037e, 0x00037f, 0x000380, 
	0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003f6, 0x0003f7, 0x000482, 0x00048a, 0x000530, 0x000531, 0x000557, 
	0x000559, 0x00055a, 0x000560, 0x000589, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000620, 0x00064b, 0x00066e, 0x000670, 0x000671, 0x0006d4, 0x0006d5, 0x0006d6, 
	0x0006e5, 0x0006e7, 0x0006ee, 0x0006f0, 0x0006fa, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x000711, 0x000712, 0x000730, 0x00074d, 0x0007a6, 0x0007b1, 0x0007b2, 
	0x0007ca, 0x0007eb, 0x0007f4, 0x0007f6, 0x0007fa, 0x0007fb, 0x000800, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x000840, 0x000859, 
	0x000860, 0x00086b, 0x000870, 0x000888, 0x000889, 0x00088f, 0x0008a0, 0x0008ca, 0x000904, 0x00093a, 0x00093d, 0x00093e, 0x000950, 0x000951, 0x000958, 0x000962, 
	0x000971, 0x000981, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009be, 
	0x0009ce, 0x0009cf, 0x0009dc, 0x0009de, 0x0009df, 0x0009e2, 0x0009f0, 0x0009f2, 0x0009fc, 0x0009fd, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 
	0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a72, 0x000a75, 0x000a85, 0x000a8e, 
	0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000abe, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae2, 
	0x000af9, 0x000afa, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3d, 0x000b3e, 
	0x000b5c, 0x000b5e, 0x000b5f, 0x000b62, 0x000b71, 0x000b72, 0x000b83, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 
	0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bd0, 0x000bd1, 0x000c05, 0x000c0d, 0x000c0e, 0x000c11, 
	0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3d, 0x000c3e, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c62, 0x000c80, 0x000c81, 0x000c85, 0x000c8d, 
	0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cbe, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce2, 0x000cf1, 0x000cf3, 
	0x000d04, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d4e, 0x000d4f, 0x000d54, 0x000d57, 0x000d5f, 0x000d62, 0x000d7a, 0x000d80, 
	0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000e01, 0x000e31, 0x000e32, 0x000e34, 0x000e40, 0x000e47, 
	0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ebe, 
	0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000edc, 0x000ee0, 0x000f00, 0x000f01, 0x000f40, 0x000f48, 0x000f49, 0x000f6d, 0x000f88, 0x000f8d, 0x001000, 0x00102b, 
	0x00103f, 0x001040, 0x001050, 0x001056, 0x00105a, 0x00105e, 0x001061, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001071, 0x001075, 0x001082, 0x00108e, 0x00108f, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 
	0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 
	0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001380, 0x001390, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001401, 0x00166d, 
	0x00166f, 0x001680, 0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016f1, 0x0016f9, 0x001700, 0x001712, 0x00171f, 0x001732, 0x001740, 0x001752, 0x001760, 0x00176d, 
	0x00176e, 0x001771, 0x001780, 0x0017b4, 0x0017d7, 0x0017d8, 0x0017dc, 0x0017dd, 0x001820, 0x001879, 0x001880, 0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x0018ab, 
	0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001950, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x001a00, 0x001a17, 0x001a20, 0x001a55, 
	0x001aa7, 0x001aa8, 0x001b05, 0x001b34, 0x001b45, 0x001b4d, 0x001b83, 0x001ba1, 0x001bae, 0x001bb0, 0x001bba, 0x001be6, 0x001c00, 0x001c24, 0x001c4d, 0x001c50, 
	0x001c5a, 0x001c7e, 0x001c80, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001cfb, 
	0x001d00, 0x001dc0, 0x001e00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 
	0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fcd, 0x001fd0, 0x001fd4, 
	0x001fd6, 0x001fdc, 0x001fe0, 0x001fed, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ffd, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x002102, 0x002103, 
	0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 
	0x00212f, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002183, 0x002185, 0x002c00, 0x002ce5, 0x002ceb, 0x002cef, 0x002cf2, 0x002cf4, 
	0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d70, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 
	0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x002e2f, 0x002e30, 0x003005, 0x003007, 
	0x003031, 0x003036, 0x00303b, 0x00303d, 0x003041, 0x003097, 0x00309d, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 
	0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 0x003400, 0x004dc0, 0x004e00, 0x00a48d, 0x00a4d0, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a620, 0x00a62a, 0x00a62c, 
	0x00a640, 0x00a66f, 0x00a67f, 0x00a69e, 0x00a6a0, 0x00a6e6, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 
	0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a823, 0x00a840, 0x00a874, 0x00a882, 0x00a8b4, 0x00a8f2, 0x00a8f8, 
	0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a8ff, 0x00a90a, 0x00a926, 0x00a930, 0x00a947, 0x00a960, 0x00a97d, 0x00a984, 0x00a9b3, 0x00a9cf, 0x00a9d0, 0x00a9e0, 0x00a9e5, 
	0x00a9e6, 0x00a9f0, 0x00a9fa, 0x00a9ff, 0x00aa00, 0x00aa29, 0x00aa40, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa60, 0x00aa77, 0x00aa7a, 0x00aa7b, 0x00aa7e, 0x00aab0, 
	0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 0x00aadb, 0x00aade, 0x00aae0, 0x00aaeb, 0x00aaf2, 0x00aaf5, 
	0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab70, 0x00abe3, 
	0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb1e, 
	0x00fb1f, 0x00fb29, 0x00fb2a, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 
	0x00fd50, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdf0, 0x00fdfc, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ffbf, 
	0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 
	0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x010300, 0x010320, 0x01032d, 0x010341, 0x010342, 0x01034a, 
	0x010350, 0x010376, 0x010380, 0x01039e, 0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x010400, 0x01049e, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 
	0x010530, 0x010564, 0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 
	0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 
	0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 0x0108e0, 0x0108f3, 
	0x0108f4, 0x0108f6, 0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 0x010a00, 0x010a01, 0x010a10, 0x010a14, 0x010a15, 0x010a18, 
	0x010a19, 0x010a36, 0x010a60, 0x010a7d, 0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae5, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 0x010b60, 0x010b73, 
	0x010b80, 0x010b92, 0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010d00, 0x010d24, 0x010e80, 0x010eaa, 0x010eb0, 0x010eb2, 0x010f00, 0x010f1d, 
	0x010f27, 0x010f28, 0x010f30, 0x010f46, 0x010f70, 0x010f82, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011003, 0x011038, 0x011071, 0x011073, 0x011075, 0x011076, 
	0x011083, 0x0110b0, 0x0110d0, 0x0110e9, 0x011103, 0x011127, 0x011144, 0x011145, 0x011147, 0x011148, 0x011150, 0x011173, 0x011176, 0x011177, 0x011183, 0x0111b3, 
	0x0111c1, 0x0111c5, 0x0111da, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x01122c, 0x01123f, 0x011241, 0x011280, 0x011287, 0x011288, 0x011289, 
	0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112df, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 
	0x011332, 0x011334, 0x011335, 0x01133a, 0x01133d, 0x01133e, 0x011350, 0x011351, 0x01135d, 0x011362, 0x011400, 0x011435, 0x011447, 0x01144b, 0x01145f, 0x011462, 
	0x011480, 0x0114b0, 0x0114c4, 0x0114c6, 0x0114c7, 0x0114c8, 0x011580, 0x0115af, 0x0115d8, 0x0115dc, 0x011600, 0x011630, 0x011644, 0x011645, 0x011680, 0x0116ab, 
	0x0116b8, 0x0116b9, 0x011700, 0x01171b, 0x011740, 0x011747, 0x011800, 0x01182c, 0x0118a0, 0x0118e0, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 
	0x011915, 0x011917, 0x011918, 0x011930, 0x01193f, 0x011940, 0x011941, 0x011942, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d1, 0x0119e1, 0x0119e2, 0x0119e3, 0x0119e4, 
	0x011a00, 0x011a01, 0x011a0b, 0x011a33, 0x011a3a, 0x011a3b, 0x011a50, 0x011a51, 0x011a5c, 0x011a8a, 0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 
	0x011c0a, 0x011c2f, 0x011c40, 0x011c41, 0x011c72, 0x011c90, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d31, 0x011d46, 0x011d47, 0x011d60, 0x011d66, 
	0x011d67, 0x011d69, 0x011d6a, 0x011d8a, 0x011d98, 0x011d99, 0x011ee0, 0x011ef3, 0x011f02, 0x011f03, 0x011f04, 0x011f11, 0x011f12, 0x011f34, 0x011fb0, 0x011fb1, 
	0x012000, 0x01239a, 0x012480, 0x012544, 0x012f90, 0x012ff1, 0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 
	0x016a70, 0x016abf, 0x016ad0, 0x016aee, 0x016b00, 0x016b30, 0x016b40, 0x016b44, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e80, 0x016f00, 0x016f4b, 
	0x016f50, 0x016f51, 0x016f93, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe4, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 
	0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 
	0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 
	0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 
	0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d6c1, 
	0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 
	0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7cc, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e030, 0x01e06e, 0x01e100, 0x01e12d, 0x01e137, 0x01e13e, 0x01e14e, 0x01e14f, 
	0x01e290, 0x01e2ae, 0x01e2c0, 0x01e2ec, 0x01e4d0, 0x01e4ec, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 
	0x01e900, 0x01e944, 0x01e94b, 0x01e94c, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 
	0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 
	0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 
	0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 
	0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 
	0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_General_Category_Letter_Number 24
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Letter_Number[mxCharSet_General_Category_Letter_Number] = {
	0x0016ee, 0x0016f1, 0x002160, 0x002183, 0x002185, 0x002189, 0x003007, 0x003008, 0x003021, 0x00302a, 0x003038, 0x00303b, 0x00a6e6, 0x00a6f0, 0x010140, 0x010175, 
	0x010341, 0x010342, 0x01034a, 0x01034b, 0x0103d1, 0x0103d6, 0x012400, 0x01246f, 
};
#define mxCharSet_General_Category_Line_Separator 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Line_Separator[mxCharSet_General_Category_Line_Separator] = {
	0x002028, 0x002029, 
};
#define mxCharSet_General_Category_Lowercase_Letter 1316
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Lowercase_Letter[mxCharSet_General_Category_Lowercase_Letter] = {
	0x000061, 0x00007b, 0x0000b5, 0x0000b6, 0x0000df, 0x0000f7, 0x0000f8, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 
	0x000109, 0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 
	0x000119, 0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 
	0x000129, 0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000139, 
	0x00013a, 0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 
	0x00014b, 0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 
	0x00015b, 0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 
	0x00016b, 0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 
	0x00017c, 0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 0x000193, 0x000195, 0x000196, 
	0x000199, 0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 
	0x0001b0, 0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bb, 0x0001bd, 0x0001c0, 0x0001c6, 0x0001c7, 0x0001c9, 0x0001ca, 0x0001cc, 0x0001cd, 
	0x0001ce, 0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 
	0x0001df, 0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 
	0x0001ef, 0x0001f1, 0x0001f3, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 
	0x000203, 0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 
	0x000213, 0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 0x000221, 0x000222, 
	0x000223, 0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 
	0x000233, 0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 
	0x00024f, 0x000294, 0x000295, 0x0002b0, 0x000371, 0x000372, 0x000373, 0x000374, 0x000377, 0x000378, 0x00037b, 0x00037e, 0x000390, 0x000391, 0x0003ac, 0x0003cf, 
	0x0003d0, 0x0003d2, 0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 
	0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 0x0003f4, 0x0003f5, 0x0003f6, 0x0003f8, 0x0003f9, 
	0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 
	0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 
	0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x000482, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 
	0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 
	0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 
	0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 
	0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 
	0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 
	0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 
	0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 
	0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 
	0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 
	0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000530, 0x000560, 0x000589, 0x0010d0, 0x0010fb, 0x0010fd, 0x001100, 
	0x0013f8, 0x0013fe, 0x001c80, 0x001c89, 0x001d00, 0x001d2c, 0x001d6b, 0x001d78, 0x001d79, 0x001d9b, 0x001e01, 0x001e02, 0x001e03, 0x001e04, 0x001e05, 0x001e06, 
	0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 0x001e14, 0x001e15, 0x001e16, 
	0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 0x001e24, 0x001e25, 0x001e26, 
	0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 0x001e34, 0x001e35, 0x001e36, 
	0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 0x001e44, 0x001e45, 0x001e46, 
	0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 0x001e54, 0x001e55, 0x001e56, 
	0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 0x001e64, 0x001e65, 0x001e66, 
	0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 0x001e74, 0x001e75, 0x001e76, 
	0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 0x001e84, 0x001e85, 0x001e86, 
	0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 0x001e94, 0x001e95, 0x001e9e, 
	0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 0x001eac, 0x001ead, 0x001eae, 
	0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 0x001ebc, 0x001ebd, 0x001ebe, 
	0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 0x001ecc, 0x001ecd, 0x001ece, 
	0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 0x001edc, 0x001edd, 0x001ede, 
	0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 0x001eec, 0x001eed, 0x001eee, 
	0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 0x001efc, 0x001efd, 0x001efe, 
	0x001eff, 0x001f08, 0x001f10, 0x001f16, 0x001f20, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f46, 0x001f50, 0x001f58, 0x001f60, 0x001f68, 0x001f70, 0x001f7e, 
	0x001f80, 0x001f88, 0x001f90, 0x001f98, 0x001fa0, 0x001fa8, 0x001fb0, 0x001fb5, 0x001fb6, 0x001fb8, 0x001fbe, 0x001fbf, 0x001fc2, 0x001fc5, 0x001fc6, 0x001fc8, 
	0x001fd0, 0x001fd4, 0x001fd6, 0x001fd8, 0x001fe0, 0x001fe8, 0x001ff2, 0x001ff5, 0x001ff6, 0x001ff8, 0x00210a, 0x00210b, 0x00210e, 0x002110, 0x002113, 0x002114, 
	0x00212f, 0x002130, 0x002134, 0x002135, 0x002139, 0x00213a, 0x00213c, 0x00213e, 0x002146, 0x00214a, 0x00214e, 0x00214f, 0x002184, 0x002185, 0x002c30, 0x002c60, 
	0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 0x002c72, 0x002c73, 0x002c75, 0x002c76, 0x002c7c, 
	0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 0x002c90, 
	0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 0x002ca0, 
	0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 0x002cb0, 
	0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 0x002cc0, 
	0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 0x002cd0, 
	0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 0x002ce0, 
	0x002ce1, 0x002ce2, 0x002ce3, 0x002ce5, 0x002cec, 0x002ced, 0x002cee, 0x002cef, 0x002cf3, 0x002cf4, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 
	0x00a641, 0x00a642, 0x00a643, 0x00a644, 0x00a645, 0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 
	0x00a651, 0x00a652, 0x00a653, 0x00a654, 0x00a655, 0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 
	0x00a661, 0x00a662, 0x00a663, 0x00a664, 0x00a665, 0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a66e, 0x00a681, 0x00a682, 
	0x00a683, 0x00a684, 0x00a685, 0x00a686, 0x00a687, 0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 
	0x00a693, 0x00a694, 0x00a695, 0x00a696, 0x00a697, 0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a69c, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 
	0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 
	0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 
	0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 
	0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 
	0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 0x00a770, 0x00a771, 0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 
	0x00a783, 0x00a784, 0x00a785, 0x00a786, 0x00a787, 0x00a788, 0x00a78c, 0x00a78d, 0x00a78e, 0x00a78f, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 0x00a798, 
	0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 
	0x00a7a9, 0x00a7aa, 0x00a7af, 0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 
	0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 0x00a7cb, 0x00a7d1, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7d6, 0x00a7d7, 0x00a7d8, 
	0x00a7d9, 0x00a7da, 0x00a7f6, 0x00a7f7, 0x00a7fa, 0x00a7fb, 0x00ab30, 0x00ab5b, 0x00ab60, 0x00ab69, 0x00ab70, 0x00abc0, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 
	0x00ff41, 0x00ff5b, 0x010428, 0x010450, 0x0104d8, 0x0104fc, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010cc0, 0x010cf3, 
	0x0118c0, 0x0118e0, 0x016e60, 0x016e80, 0x01d41a, 0x01d434, 0x01d44e, 0x01d455, 0x01d456, 0x01d468, 0x01d482, 0x01d49c, 0x01d4b6, 0x01d4ba, 0x01d4bb, 0x01d4bc, 
	0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d4d0, 0x01d4ea, 0x01d504, 0x01d51e, 0x01d538, 0x01d552, 0x01d56c, 0x01d586, 0x01d5a0, 0x01d5ba, 0x01d5d4, 0x01d5ee, 0x01d608, 
	0x01d622, 0x01d63c, 0x01d656, 0x01d670, 0x01d68a, 0x01d6a6, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6e2, 0x01d6fc, 0x01d715, 0x01d716, 0x01d71c, 0x01d736, 0x01d74f, 
	0x01d750, 0x01d756, 0x01d770, 0x01d789, 0x01d78a, 0x01d790, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01d7ca, 0x01d7cb, 0x01d7cc, 0x01df00, 0x01df0a, 0x01df0b, 0x01df1f, 
	0x01df25, 0x01df2b, 0x01e922, 0x01e944, 
};
#define mxCharSet_General_Category_Mark 620
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Mark[mxCharSet_General_Category_Mark] = {
	0x000300, 0x000370, 0x000483, 0x00048a, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x000610, 0x00061b, 
	0x00064b, 0x000660, 0x000670, 0x000671, 0x0006d6, 0x0006dd, 0x0006df, 0x0006e5, 0x0006e7, 0x0006e9, 0x0006ea, 0x0006ee, 0x000711, 0x000712, 0x000730, 0x00074b, 
	0x0007a6, 0x0007b1, 0x0007eb, 0x0007f4, 0x0007fd, 0x0007fe, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x00082e, 0x000859, 0x00085c, 
	0x000898, 0x0008a0, 0x0008ca, 0x0008e2, 0x0008e3, 0x000904, 0x00093a, 0x00093d, 0x00093e, 0x000950, 0x000951, 0x000958, 0x000962, 0x000964, 0x000981, 0x000984, 
	0x0009bc, 0x0009bd, 0x0009be, 0x0009c5, 0x0009c7, 0x0009c9, 0x0009cb, 0x0009ce, 0x0009d7, 0x0009d8, 0x0009e2, 0x0009e4, 0x0009fe, 0x0009ff, 0x000a01, 0x000a04, 
	0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a70, 0x000a72, 0x000a75, 0x000a76, 0x000a81, 0x000a84, 
	0x000abc, 0x000abd, 0x000abe, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ae2, 0x000ae4, 0x000afa, 0x000b00, 0x000b01, 0x000b04, 0x000b3c, 0x000b3d, 
	0x000b3e, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b62, 0x000b64, 0x000b82, 0x000b83, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 
	0x000bca, 0x000bce, 0x000bd7, 0x000bd8, 0x000c00, 0x000c05, 0x000c3c, 0x000c3d, 0x000c3e, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 
	0x000c62, 0x000c64, 0x000c81, 0x000c84, 0x000cbc, 0x000cbd, 0x000cbe, 0x000cc5, 0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000ce2, 0x000ce4, 
	0x000cf3, 0x000cf4, 0x000d00, 0x000d04, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d4e, 0x000d57, 0x000d58, 0x000d62, 0x000d64, 
	0x000d81, 0x000d84, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd5, 0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000df2, 0x000df4, 0x000e31, 0x000e32, 0x000e34, 0x000e3b, 
	0x000e47, 0x000e4f, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ec8, 0x000ecf, 0x000f18, 0x000f1a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000f3a, 
	0x000f3e, 0x000f40, 0x000f71, 0x000f85, 0x000f86, 0x000f88, 0x000f8d, 0x000f98, 0x000f99, 0x000fbd, 0x000fc6, 0x000fc7, 0x00102b, 0x00103f, 0x001056, 0x00105a, 
	0x00105e, 0x001061, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001071, 0x001075, 0x001082, 0x00108e, 0x00108f, 0x001090, 0x00109a, 0x00109e, 0x00135d, 0x001360, 
	0x001712, 0x001716, 0x001732, 0x001735, 0x001752, 0x001754, 0x001772, 0x001774, 0x0017b4, 0x0017d4, 0x0017dd, 0x0017de, 0x00180b, 0x00180e, 0x00180f, 0x001810, 
	0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x001920, 0x00192c, 0x001930, 0x00193c, 0x001a17, 0x001a1c, 0x001a55, 0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 0x001a80, 
	0x001ab0, 0x001acf, 0x001b00, 0x001b05, 0x001b34, 0x001b45, 0x001b6b, 0x001b74, 0x001b80, 0x001b83, 0x001ba1, 0x001bae, 0x001be6, 0x001bf4, 0x001c24, 0x001c38, 
	0x001cd0, 0x001cd3, 0x001cd4, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001dc0, 0x001e00, 0x0020d0, 0x0020f1, 0x002cef, 0x002cf2, 
	0x002d7f, 0x002d80, 0x002de0, 0x002e00, 0x00302a, 0x003030, 0x003099, 0x00309b, 0x00a66f, 0x00a673, 0x00a674, 0x00a67e, 0x00a69e, 0x00a6a0, 0x00a6f0, 0x00a6f2, 
	0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a823, 0x00a828, 0x00a82c, 0x00a82d, 0x00a880, 0x00a882, 0x00a8b4, 0x00a8c6, 0x00a8e0, 0x00a8f2, 
	0x00a8ff, 0x00a900, 0x00a926, 0x00a92e, 0x00a947, 0x00a954, 0x00a980, 0x00a984, 0x00a9b3, 0x00a9c1, 0x00a9e5, 0x00a9e6, 0x00aa29, 0x00aa37, 0x00aa43, 0x00aa44, 
	0x00aa4c, 0x00aa4e, 0x00aa7b, 0x00aa7e, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aaeb, 0x00aaf0, 
	0x00aaf5, 0x00aaf7, 0x00abe3, 0x00abeb, 0x00abec, 0x00abee, 0x00fb1e, 0x00fb1f, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe30, 0x0101fd, 0x0101fe, 0x0102e0, 0x0102e1, 
	0x010376, 0x01037b, 0x010a01, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a10, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 0x010ae5, 0x010ae7, 0x010d24, 0x010d28, 
	0x010eab, 0x010ead, 0x010efd, 0x010f00, 0x010f46, 0x010f51, 0x010f82, 0x010f86, 0x011000, 0x011003, 0x011038, 0x011047, 0x011070, 0x011071, 0x011073, 0x011075, 
	0x01107f, 0x011083, 0x0110b0, 0x0110bb, 0x0110c2, 0x0110c3, 0x011100, 0x011103, 0x011127, 0x011135, 0x011145, 0x011147, 0x011173, 0x011174, 0x011180, 0x011183, 
	0x0111b3, 0x0111c1, 0x0111c9, 0x0111cd, 0x0111ce, 0x0111d0, 0x01122c, 0x011238, 0x01123e, 0x01123f, 0x011241, 0x011242, 0x0112df, 0x0112eb, 0x011300, 0x011304, 
	0x01133b, 0x01133d, 0x01133e, 0x011345, 0x011347, 0x011349, 0x01134b, 0x01134e, 0x011357, 0x011358, 0x011362, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 
	0x011435, 0x011447, 0x01145e, 0x01145f, 0x0114b0, 0x0114c4, 0x0115af, 0x0115b6, 0x0115b8, 0x0115c1, 0x0115dc, 0x0115de, 0x011630, 0x011641, 0x0116ab, 0x0116b8, 
	0x01171d, 0x01172c, 0x01182c, 0x01183b, 0x011930, 0x011936, 0x011937, 0x011939, 0x01193b, 0x01193f, 0x011940, 0x011941, 0x011942, 0x011944, 0x0119d1, 0x0119d8, 
	0x0119da, 0x0119e1, 0x0119e4, 0x0119e5, 0x011a01, 0x011a0b, 0x011a33, 0x011a3a, 0x011a3b, 0x011a3f, 0x011a47, 0x011a48, 0x011a51, 0x011a5c, 0x011a8a, 0x011a9a, 
	0x011c2f, 0x011c37, 0x011c38, 0x011c40, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 0x011d31, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d46, 
	0x011d47, 0x011d48, 0x011d8a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d98, 0x011ef3, 0x011ef7, 0x011f00, 0x011f02, 0x011f03, 0x011f04, 0x011f34, 0x011f3b, 
	0x011f3e, 0x011f43, 0x013440, 0x013441, 0x013447, 0x013456, 0x016af0, 0x016af5, 0x016b30, 0x016b37, 0x016f4f, 0x016f50, 0x016f51, 0x016f88, 0x016f8f, 0x016f93, 
	0x016fe4, 0x016fe5, 0x016ff0, 0x016ff2, 0x01bc9d, 0x01bc9f, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01d165, 0x01d16a, 0x01d16d, 0x01d173, 0x01d17b, 0x01d183, 
	0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 0x01da00, 0x01da37, 0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 
	0x01daa1, 0x01dab0, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e08f, 0x01e090, 0x01e130, 0x01e137, 
	0x01e2ae, 0x01e2af, 0x01e2ec, 0x01e2f0, 0x01e4ec, 0x01e4f0, 0x01e8d0, 0x01e8d7, 0x01e944, 0x01e94b, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_General_Category_Math_Symbol 128
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Math_Symbol[mxCharSet_General_Category_Math_Symbol] = {
	0x00002b, 0x00002c, 0x00003c, 0x00003f, 0x00007c, 0x00007d, 0x00007e, 0x00007f, 0x0000ac, 0x0000ad, 0x0000b1, 0x0000b2, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 
	0x0003f6, 0x0003f7, 0x000606, 0x000609, 0x002044, 0x002045, 0x002052, 0x002053, 0x00207a, 0x00207d, 0x00208a, 0x00208d, 0x002118, 0x002119, 0x002140, 0x002145, 
	0x00214b, 0x00214c, 0x002190, 0x002195, 0x00219a, 0x00219c, 0x0021a0, 0x0021a1, 0x0021a3, 0x0021a4, 0x0021a6, 0x0021a7, 0x0021ae, 0x0021af, 0x0021ce, 0x0021d0, 
	0x0021d2, 0x0021d3, 0x0021d4, 0x0021d5, 0x0021f4, 0x002300, 0x002320, 0x002322, 0x00237c, 0x00237d, 0x00239b, 0x0023b4, 0x0023dc, 0x0023e2, 0x0025b7, 0x0025b8, 
	0x0025c1, 0x0025c2, 0x0025f8, 0x002600, 0x00266f, 0x002670, 0x0027c0, 0x0027c5, 0x0027c7, 0x0027e6, 0x0027f0, 0x002800, 0x002900, 0x002983, 0x002999, 0x0029d8, 
	0x0029dc, 0x0029fc, 0x0029fe, 0x002b00, 0x002b30, 0x002b45, 0x002b47, 0x002b4d, 0x00fb29, 0x00fb2a, 0x00fe62, 0x00fe63, 0x00fe64, 0x00fe67, 0x00ff0b, 0x00ff0c, 
	0x00ff1c, 0x00ff1f, 0x00ff5c, 0x00ff5d, 0x00ff5e, 0x00ff5f, 0x00ffe2, 0x00ffe3, 0x00ffe9, 0x00ffed, 0x01d6c1, 0x01d6c2, 0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 
	0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 0x01d7c3, 0x01d7c4, 0x01eef0, 0x01eef2, 
};
#define mxCharSet_General_Category_Modifier_Letter 142
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Modifier_Letter[mxCharSet_General_Category_Modifier_Letter] = {
	0x0002b0, 0x0002c2, 0x0002c6, 0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000374, 0x000375, 0x00037a, 0x00037b, 0x000559, 0x00055a, 
	0x000640, 0x000641, 0x0006e5, 0x0006e7, 0x0007f4, 0x0007f6, 0x0007fa, 0x0007fb, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x0008c9, 0x0008ca, 
	0x000971, 0x000972, 0x000e46, 0x000e47, 0x000ec6, 0x000ec7, 0x0010fc, 0x0010fd, 0x0017d7, 0x0017d8, 0x001843, 0x001844, 0x001aa7, 0x001aa8, 0x001c78, 0x001c7e, 
	0x001d2c, 0x001d6b, 0x001d78, 0x001d79, 0x001d9b, 0x001dc0, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x002c7c, 0x002c7e, 0x002d6f, 0x002d70, 
	0x002e2f, 0x002e30, 0x003005, 0x003006, 0x003031, 0x003036, 0x00303b, 0x00303c, 0x00309d, 0x00309f, 0x0030fc, 0x0030ff, 0x00a015, 0x00a016, 0x00a4f8, 0x00a4fe, 
	0x00a60c, 0x00a60d, 0x00a67f, 0x00a680, 0x00a69c, 0x00a69e, 0x00a717, 0x00a720, 0x00a770, 0x00a771, 0x00a788, 0x00a789, 0x00a7f2, 0x00a7f5, 0x00a7f8, 0x00a7fa, 
	0x00a9cf, 0x00a9d0, 0x00a9e6, 0x00a9e7, 0x00aa70, 0x00aa71, 0x00aadd, 0x00aade, 0x00aaf3, 0x00aaf5, 0x00ab5c, 0x00ab60, 0x00ab69, 0x00ab6a, 0x00ff70, 0x00ff71, 
	0x00ff9e, 0x00ffa0, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x016b40, 0x016b44, 0x016f93, 0x016fa0, 0x016fe0, 0x016fe2, 0x016fe3, 0x016fe4, 
	0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01e030, 0x01e06e, 0x01e137, 0x01e13e, 0x01e4eb, 0x01e4ec, 0x01e94b, 0x01e94c, 
};
#define mxCharSet_General_Category_Modifier_Symbol 62
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Modifier_Symbol[mxCharSet_General_Category_Modifier_Symbol] = {
	0x00005e, 0x00005f, 0x000060, 0x000061, 0x0000a8, 0x0000a9, 0x0000af, 0x0000b0, 0x0000b4, 0x0000b5, 0x0000b8, 0x0000b9, 0x0002c2, 0x0002c6, 0x0002d2, 0x0002e0, 
	0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000300, 0x000375, 0x000376, 0x000384, 0x000386, 0x000888, 0x000889, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 
	0x001fcd, 0x001fd0, 0x001fdd, 0x001fe0, 0x001fed, 0x001ff0, 0x001ffd, 0x001fff, 0x00309b, 0x00309d, 0x00a700, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 
	0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab6c, 0x00fbb2, 0x00fbc3, 0x00ff3e, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ffe3, 0x00ffe4, 0x01f3fb, 0x01f400, 
};
#define mxCharSet_General_Category_Nonspacing_Mark 692
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Nonspacing_Mark[mxCharSet_General_Category_Nonspacing_Mark] = {
	0x000300, 0x000370, 0x000483, 0x000488, 0x000591, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005c8, 0x000610, 0x00061b, 
	0x00064b, 0x000660, 0x000670, 0x000671, 0x0006d6, 0x0006dd, 0x0006df, 0x0006e5, 0x0006e7, 0x0006e9, 0x0006ea, 0x0006ee, 0x000711, 0x000712, 0x000730, 0x00074b, 
	0x0007a6, 0x0007b1, 0x0007eb, 0x0007f4, 0x0007fd, 0x0007fe, 0x000816, 0x00081a, 0x00081b, 0x000824, 0x000825, 0x000828, 0x000829, 0x00082e, 0x000859, 0x00085c, 
	0x000898, 0x0008a0, 0x0008ca, 0x0008e2, 0x0008e3, 0x000903, 0x00093a, 0x00093b, 0x00093c, 0x00093d, 0x000941, 0x000949, 0x00094d, 0x00094e, 0x000951, 0x000958, 
	0x000962, 0x000964, 0x000981, 0x000982, 0x0009bc, 0x0009bd, 0x0009c1, 0x0009c5, 0x0009cd, 0x0009ce, 0x0009e2, 0x0009e4, 0x0009fe, 0x0009ff, 0x000a01, 0x000a03, 
	0x000a3c, 0x000a3d, 0x000a41, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a70, 0x000a72, 0x000a75, 0x000a76, 0x000a81, 0x000a83, 
	0x000abc, 0x000abd, 0x000ac1, 0x000ac6, 0x000ac7, 0x000ac9, 0x000acd, 0x000ace, 0x000ae2, 0x000ae4, 0x000afa, 0x000b00, 0x000b01, 0x000b02, 0x000b3c, 0x000b3d, 
	0x000b3f, 0x000b40, 0x000b41, 0x000b45, 0x000b4d, 0x000b4e, 0x000b55, 0x000b57, 0x000b62, 0x000b64, 0x000b82, 0x000b83, 0x000bc0, 0x000bc1, 0x000bcd, 0x000bce, 
	0x000c00, 0x000c01, 0x000c04, 0x000c05, 0x000c3c, 0x000c3d, 0x000c3e, 0x000c41, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c62, 0x000c64, 
	0x000c81, 0x000c82, 0x000cbc, 0x000cbd, 0x000cbf, 0x000cc0, 0x000cc6, 0x000cc7, 0x000ccc, 0x000cce, 0x000ce2, 0x000ce4, 0x000d00, 0x000d02, 0x000d3b, 0x000d3d, 
	0x000d41, 0x000d45, 0x000d4d, 0x000d4e, 0x000d62, 0x000d64, 0x000d81, 0x000d82, 0x000dca, 0x000dcb, 0x000dd2, 0x000dd5, 0x000dd6, 0x000dd7, 0x000e31, 0x000e32, 
	0x000e34, 0x000e3b, 0x000e47, 0x000e4f, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ec8, 0x000ecf, 0x000f18, 0x000f1a, 0x000f35, 0x000f36, 0x000f37, 0x000f38, 
	0x000f39, 0x000f3a, 0x000f71, 0x000f7f, 0x000f80, 0x000f85, 0x000f86, 0x000f88, 0x000f8d, 0x000f98, 0x000f99, 0x000fbd, 0x000fc6, 0x000fc7, 0x00102d, 0x001031, 
	0x001032, 0x001038, 0x001039, 0x00103b, 0x00103d, 0x00103f, 0x001058, 0x00105a, 0x00105e, 0x001061, 0x001071, 0x001075, 0x001082, 0x001083, 0x001085, 0x001087, 
	0x00108d, 0x00108e, 0x00109d, 0x00109e, 0x00135d, 0x001360, 0x001712, 0x001715, 0x001732, 0x001734, 0x001752, 0x001754, 0x001772, 0x001774, 0x0017b4, 0x0017b6, 
	0x0017b7, 0x0017be, 0x0017c6, 0x0017c7, 0x0017c9, 0x0017d4, 0x0017dd, 0x0017de, 0x00180b, 0x00180e, 0x00180f, 0x001810, 0x001885, 0x001887, 0x0018a9, 0x0018aa, 
	0x001920, 0x001923, 0x001927, 0x001929, 0x001932, 0x001933, 0x001939, 0x00193c, 0x001a17, 0x001a19, 0x001a1b, 0x001a1c, 0x001a56, 0x001a57, 0x001a58, 0x001a5f, 
	0x001a60, 0x001a61, 0x001a62, 0x001a63, 0x001a65, 0x001a6d, 0x001a73, 0x001a7d, 0x001a7f, 0x001a80, 0x001ab0, 0x001abe, 0x001abf, 0x001acf, 0x001b00, 0x001b04, 
	0x001b34, 0x001b35, 0x001b36, 0x001b3b, 0x001b3c, 0x001b3d, 0x001b42, 0x001b43, 0x001b6b, 0x001b74, 0x001b80, 0x001b82, 0x001ba2, 0x001ba6, 0x001ba8, 0x001baa, 
	0x001bab, 0x001bae, 0x001be6, 0x001be7, 0x001be8, 0x001bea, 0x001bed, 0x001bee, 0x001bef, 0x001bf2, 0x001c2c, 0x001c34, 0x001c36, 0x001c38, 0x001cd0, 0x001cd3, 
	0x001cd4, 0x001ce1, 0x001ce2, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001dc0, 0x001e00, 0x0020d0, 0x0020dd, 0x0020e1, 0x0020e2, 
	0x0020e5, 0x0020f1, 0x002cef, 0x002cf2, 0x002d7f, 0x002d80, 0x002de0, 0x002e00, 0x00302a, 0x00302e, 0x003099, 0x00309b, 0x00a66f, 0x00a670, 0x00a674, 0x00a67e, 
	0x00a69e, 0x00a6a0, 0x00a6f0, 0x00a6f2, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a825, 0x00a827, 0x00a82c, 0x00a82d, 0x00a8c4, 0x00a8c6, 
	0x00a8e0, 0x00a8f2, 0x00a8ff, 0x00a900, 0x00a926, 0x00a92e, 0x00a947, 0x00a952, 0x00a980, 0x00a983, 0x00a9b3, 0x00a9b4, 0x00a9b6, 0x00a9ba, 0x00a9bc, 0x00a9be, 
	0x00a9e5, 0x00a9e6, 0x00aa29, 0x00aa2f, 0x00aa31, 0x00aa33, 0x00aa35, 0x00aa37, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa4d, 0x00aa7c, 0x00aa7d, 0x00aab0, 0x00aab1, 
	0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aaec, 0x00aaee, 0x00aaf6, 0x00aaf7, 0x00abe5, 0x00abe6, 0x00abe8, 0x00abe9, 
	0x00abed, 0x00abee, 0x00fb1e, 0x00fb1f, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe30, 0x0101fd, 0x0101fe, 0x0102e0, 0x0102e1, 0x010376, 0x01037b, 0x010a01, 0x010a04, 
	0x010a05, 0x010a07, 0x010a0c, 0x010a10, 0x010a38, 0x010a3b, 0x010a3f, 0x010a40, 0x010ae5, 0x010ae7, 0x010d24, 0x010d28, 0x010eab, 0x010ead, 0x010efd, 0x010f00, 
	0x010f46, 0x010f51, 0x010f82, 0x010f86, 0x011001, 0x011002, 0x011038, 0x011047, 0x011070, 0x011071, 0x011073, 0x011075, 0x01107f, 0x011082, 0x0110b3, 0x0110b7, 
	0x0110b9, 0x0110bb, 0x0110c2, 0x0110c3, 0x011100, 0x011103, 0x011127, 0x01112c, 0x01112d, 0x011135, 0x011173, 0x011174, 0x011180, 0x011182, 0x0111b6, 0x0111bf, 
	0x0111c9, 0x0111cd, 0x0111cf, 0x0111d0, 0x01122f, 0x011232, 0x011234, 0x011235, 0x011236, 0x011238, 0x01123e, 0x01123f, 0x011241, 0x011242, 0x0112df, 0x0112e0, 
	0x0112e3, 0x0112eb, 0x011300, 0x011302, 0x01133b, 0x01133d, 0x011340, 0x011341, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011438, 0x011440, 0x011442, 0x011445, 
	0x011446, 0x011447, 0x01145e, 0x01145f, 0x0114b3, 0x0114b9, 0x0114ba, 0x0114bb, 0x0114bf, 0x0114c1, 0x0114c2, 0x0114c4, 0x0115b2, 0x0115b6, 0x0115bc, 0x0115be, 
	0x0115bf, 0x0115c1, 0x0115dc, 0x0115de, 0x011633, 0x01163b, 0x01163d, 0x01163e, 0x01163f, 0x011641, 0x0116ab, 0x0116ac, 0x0116ad, 0x0116ae, 0x0116b0, 0x0116b6, 
	0x0116b7, 0x0116b8, 0x01171d, 0x011720, 0x011722, 0x011726, 0x011727, 0x01172c, 0x01182f, 0x011838, 0x011839, 0x01183b, 0x01193b, 0x01193d, 0x01193e, 0x01193f, 
	0x011943, 0x011944, 0x0119d4, 0x0119d8, 0x0119da, 0x0119dc, 0x0119e0, 0x0119e1, 0x011a01, 0x011a0b, 0x011a33, 0x011a39, 0x011a3b, 0x011a3f, 0x011a47, 0x011a48, 
	0x011a51, 0x011a57, 0x011a59, 0x011a5c, 0x011a8a, 0x011a97, 0x011a98, 0x011a9a, 0x011c30, 0x011c37, 0x011c38, 0x011c3e, 0x011c3f, 0x011c40, 0x011c92, 0x011ca8, 
	0x011caa, 0x011cb1, 0x011cb2, 0x011cb4, 0x011cb5, 0x011cb7, 0x011d31, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d46, 0x011d47, 0x011d48, 
	0x011d90, 0x011d92, 0x011d95, 0x011d96, 0x011d97, 0x011d98, 0x011ef3, 0x011ef5, 0x011f00, 0x011f02, 0x011f36, 0x011f3b, 0x011f40, 0x011f41, 0x011f42, 0x011f43, 
	0x013440, 0x013441, 0x013447, 0x013456, 0x016af0, 0x016af5, 0x016b30, 0x016b37, 0x016f4f, 0x016f50, 0x016f8f, 0x016f93, 0x016fe4, 0x016fe5, 0x01bc9d, 0x01bc9f, 
	0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01d167, 0x01d16a, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d242, 0x01d245, 0x01da00, 0x01da37, 
	0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 
	0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e08f, 0x01e090, 0x01e130, 0x01e137, 0x01e2ae, 0x01e2af, 0x01e2ec, 0x01e2f0, 0x01e4ec, 0x01e4f0, 0x01e8d0, 0x01e8d7, 
	0x01e944, 0x01e94b, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_General_Category_Number 274
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Number[mxCharSet_General_Category_Number] = {
	0x000030, 0x00003a, 0x0000b2, 0x0000b4, 0x0000b9, 0x0000ba, 0x0000bc, 0x0000bf, 0x000660, 0x00066a, 0x0006f0, 0x0006fa, 0x0007c0, 0x0007ca, 0x000966, 0x000970, 
	0x0009e6, 0x0009f0, 0x0009f4, 0x0009fa, 0x000a66, 0x000a70, 0x000ae6, 0x000af0, 0x000b66, 0x000b70, 0x000b72, 0x000b78, 0x000be6, 0x000bf3, 0x000c66, 0x000c70, 
	0x000c78, 0x000c7f, 0x000ce6, 0x000cf0, 0x000d58, 0x000d5f, 0x000d66, 0x000d79, 0x000de6, 0x000df0, 0x000e50, 0x000e5a, 0x000ed0, 0x000eda, 0x000f20, 0x000f34, 
	0x001040, 0x00104a, 0x001090, 0x00109a, 0x001369, 0x00137d, 0x0016ee, 0x0016f1, 0x0017e0, 0x0017ea, 0x0017f0, 0x0017fa, 0x001810, 0x00181a, 0x001946, 0x001950, 
	0x0019d0, 0x0019db, 0x001a80, 0x001a8a, 0x001a90, 0x001a9a, 0x001b50, 0x001b5a, 0x001bb0, 0x001bba, 0x001c40, 0x001c4a, 0x001c50, 0x001c5a, 0x002070, 0x002071, 
	0x002074, 0x00207a, 0x002080, 0x00208a, 0x002150, 0x002183, 0x002185, 0x00218a, 0x002460, 0x00249c, 0x0024ea, 0x002500, 0x002776, 0x002794, 0x002cfd, 0x002cfe, 
	0x003007, 0x003008, 0x003021, 0x00302a, 0x003038, 0x00303b, 0x003192, 0x003196, 0x003220, 0x00322a, 0x003248, 0x003250, 0x003251, 0x003260, 0x003280, 0x00328a, 
	0x0032b1, 0x0032c0, 0x00a620, 0x00a62a, 0x00a6e6, 0x00a6f0, 0x00a830, 0x00a836, 0x00a8d0, 0x00a8da, 0x00a900, 0x00a90a, 0x00a9d0, 0x00a9da, 0x00a9f0, 0x00a9fa, 
	0x00aa50, 0x00aa5a, 0x00abf0, 0x00abfa, 0x00ff10, 0x00ff1a, 0x010107, 0x010134, 0x010140, 0x010179, 0x01018a, 0x01018c, 0x0102e1, 0x0102fc, 0x010320, 0x010324, 
	0x010341, 0x010342, 0x01034a, 0x01034b, 0x0103d1, 0x0103d6, 0x0104a0, 0x0104aa, 0x010858, 0x010860, 0x010879, 0x010880, 0x0108a7, 0x0108b0, 0x0108fb, 0x010900, 
	0x010916, 0x01091c, 0x0109bc, 0x0109be, 0x0109c0, 0x0109d0, 0x0109d2, 0x010a00, 0x010a40, 0x010a49, 0x010a7d, 0x010a7f, 0x010a9d, 0x010aa0, 0x010aeb, 0x010af0, 
	0x010b58, 0x010b60, 0x010b78, 0x010b80, 0x010ba9, 0x010bb0, 0x010cfa, 0x010d00, 0x010d30, 0x010d3a, 0x010e60, 0x010e7f, 0x010f1d, 0x010f27, 0x010f51, 0x010f55, 
	0x010fc5, 0x010fcc, 0x011052, 0x011070, 0x0110f0, 0x0110fa, 0x011136, 0x011140, 0x0111d0, 0x0111da, 0x0111e1, 0x0111f5, 0x0112f0, 0x0112fa, 0x011450, 0x01145a, 
	0x0114d0, 0x0114da, 0x011650, 0x01165a, 0x0116c0, 0x0116ca, 0x011730, 0x01173c, 0x0118e0, 0x0118f3, 0x011950, 0x01195a, 0x011c50, 0x011c6d, 0x011d50, 0x011d5a, 
	0x011da0, 0x011daa, 0x011f50, 0x011f5a, 0x011fc0, 0x011fd5, 0x012400, 0x01246f, 0x016a60, 0x016a6a, 0x016ac0, 0x016aca, 0x016b50, 0x016b5a, 0x016b5b, 0x016b62, 
	0x016e80, 0x016e97, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d360, 0x01d379, 0x01d7ce, 0x01d800, 0x01e140, 0x01e14a, 0x01e2f0, 0x01e2fa, 0x01e4f0, 0x01e4fa, 
	0x01e8c7, 0x01e8d0, 0x01e950, 0x01e95a, 0x01ec71, 0x01ecac, 0x01ecad, 0x01ecb0, 0x01ecb1, 0x01ecb5, 0x01ed01, 0x01ed2e, 0x01ed2f, 0x01ed3e, 0x01f100, 0x01f10d, 
	0x01fbf0, 0x01fbfa, 
};
#define mxCharSet_General_Category_Open_Punctuation 158
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Open_Punctuation[mxCharSet_General_Category_Open_Punctuation] = {
	0x000028, 0x000029, 0x00005b, 0x00005c, 0x00007b, 0x00007c, 0x000f3a, 0x000f3b, 0x000f3c, 0x000f3d, 0x00169b, 0x00169c, 0x00201a, 0x00201b, 0x00201e, 0x00201f, 
	0x002045, 0x002046, 0x00207d, 0x00207e, 0x00208d, 0x00208e, 0x002308, 0x002309, 0x00230a, 0x00230b, 0x002329, 0x00232a, 0x002768, 0x002769, 0x00276a, 0x00276b, 
	0x00276c, 0x00276d, 0x00276e, 0x00276f, 0x002770, 0x002771, 0x002772, 0x002773, 0x002774, 0x002775, 0x0027c5, 0x0027c6, 0x0027e6, 0x0027e7, 0x0027e8, 0x0027e9, 
	0x0027ea, 0x0027eb, 0x0027ec, 0x0027ed, 0x0027ee, 0x0027ef, 0x002983, 0x002984, 0x002985, 0x002986, 0x002987, 0x002988, 0x002989, 0x00298a, 0x00298b, 0x00298c, 
	0x00298d, 0x00298e, 0x00298f, 0x002990, 0x002991, 0x002992, 0x002993, 0x002994, 0x002995, 0x002996, 0x002997, 0x002998, 0x0029d8, 0x0029d9, 0x0029da, 0x0029db, 
	0x0029fc, 0x0029fd, 0x002e22, 0x002e23, 0x002e24, 0x002e25, 0x002e26, 0x002e27, 0x002e28, 0x002e29, 0x002e42, 0x002e43, 0x002e55, 0x002e56, 0x002e57, 0x002e58, 
	0x002e59, 0x002e5a, 0x002e5b, 0x002e5c, 0x003008, 0x003009, 0x00300a, 0x00300b, 0x00300c, 0x00300d, 0x00300e, 0x00300f, 0x003010, 0x003011, 0x003014, 0x003015, 
	0x003016, 0x003017, 0x003018, 0x003019, 0x00301a, 0x00301b, 0x00301d, 0x00301e, 0x00fd3f, 0x00fd40, 0x00fe17, 0x00fe18, 0x00fe35, 0x00fe36, 0x00fe37, 0x00fe38, 
	0x00fe39, 0x00fe3a, 0x00fe3b, 0x00fe3c, 0x00fe3d, 0x00fe3e, 0x00fe3f, 0x00fe40, 0x00fe41, 0x00fe42, 0x00fe43, 0x00fe44, 0x00fe47, 0x00fe48, 0x00fe59, 0x00fe5a, 
	0x00fe5b, 0x00fe5c, 0x00fe5d, 0x00fe5e, 0x00ff08, 0x00ff09, 0x00ff3b, 0x00ff3c, 0x00ff5b, 0x00ff5c, 0x00ff5f, 0x00ff60, 0x00ff62, 0x00ff63, 
};
#define mxCharSet_General_Category_Other 1428
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Other[mxCharSet_General_Category_Other] = {
	0x000000, 0x000020, 0x00007f, 0x0000a0, 0x0000ad, 0x0000ae, 0x000378, 0x00037a, 0x000380, 0x000384, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 
	0x000530, 0x000531, 0x000557, 0x000559, 0x00058b, 0x00058d, 0x000590, 0x000591, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x000606, 0x00061c, 0x00061d, 
	0x0006dd, 0x0006de, 0x00070e, 0x000710, 0x00074b, 0x00074d, 0x0007b2, 0x0007c0, 0x0007fb, 0x0007fd, 0x00082e, 0x000830, 0x00083f, 0x000840, 0x00085c, 0x00085e, 
	0x00085f, 0x000860, 0x00086b, 0x000870, 0x00088f, 0x000898, 0x0008e2, 0x0008e3, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 
	0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 0x0009c7, 0x0009c9, 0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 
	0x0009e4, 0x0009e6, 0x0009ff, 0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 
	0x000a37, 0x000a38, 0x000a3a, 0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 
	0x000a5f, 0x000a66, 0x000a77, 0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 
	0x000aba, 0x000abc, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 0x000ae6, 0x000af2, 0x000af9, 0x000b00, 0x000b01, 
	0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 
	0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 0x000b78, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 
	0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 
	0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000be6, 0x000bfb, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 
	0x000c29, 0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 
	0x000c64, 0x000c66, 0x000c70, 0x000c77, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 0x000cc6, 
	0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 0x000d0e, 
	0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d50, 0x000d54, 0x000d64, 0x000d66, 0x000d80, 0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 
	0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd5, 0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 
	0x000df0, 0x000df2, 0x000df5, 0x000e01, 0x000e3b, 0x000e3f, 0x000e5c, 0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 
	0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f48, 0x000f49, 
	0x000f6d, 0x000f71, 0x000f98, 0x000f99, 0x000fbd, 0x000fbe, 0x000fcd, 0x000fce, 0x000fdb, 0x001000, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 
	0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 
	0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x00135d, 
	0x00137d, 0x001380, 0x00139a, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001400, 0x00169d, 0x0016a0, 0x0016f9, 0x001700, 0x001716, 0x00171f, 0x001737, 0x001740, 
	0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017de, 0x0017e0, 0x0017ea, 0x0017f0, 0x0017fa, 0x001800, 0x00180e, 0x00180f, 
	0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x00193c, 0x001940, 0x001941, 0x001944, 
	0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x001a1c, 0x001a1e, 0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 
	0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 0x001ab0, 0x001acf, 0x001b00, 0x001b4d, 0x001b50, 0x001b7f, 0x001b80, 0x001bf4, 0x001bfc, 0x001c38, 0x001c3b, 
	0x001c4a, 0x001c4d, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc8, 0x001cd0, 0x001cfb, 0x001d00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 
	0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 
	0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff6, 0x001fff, 0x002000, 0x00200b, 0x002010, 0x00202a, 0x00202f, 0x002060, 0x002070, 
	0x002072, 0x002074, 0x00208f, 0x002090, 0x00209d, 0x0020a0, 0x0020c1, 0x0020d0, 0x0020f1, 0x002100, 0x00218c, 0x002190, 0x002427, 0x002440, 0x00244b, 0x002460, 
	0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002cf4, 0x002cf9, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d71, 0x002d7f, 
	0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 
	0x002ddf, 0x002de0, 0x002e5e, 0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x002ff0, 0x002ffc, 0x003000, 0x003040, 0x003041, 0x003097, 0x003099, 
	0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x003190, 0x0031e4, 0x0031f0, 0x00321f, 0x003220, 0x00a48d, 0x00a490, 0x00a4c7, 0x00a4d0, 0x00a62c, 0x00a640, 
	0x00a6f8, 0x00a700, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a82d, 0x00a830, 0x00a83a, 0x00a840, 0x00a878, 0x00a880, 
	0x00a8c6, 0x00a8ce, 0x00a8da, 0x00a8e0, 0x00a954, 0x00a95f, 0x00a97d, 0x00a980, 0x00a9ce, 0x00a9cf, 0x00a9da, 0x00a9de, 0x00a9ff, 0x00aa00, 0x00aa37, 0x00aa40, 
	0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aac3, 0x00aadb, 0x00aaf7, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 
	0x00ab2f, 0x00ab30, 0x00ab6c, 0x00ab70, 0x00abee, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00dc00, 0x00dc00, 0x00e000, 
	0x00e000, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 
	0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbc3, 0x00fbd3, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe1a, 0x00fe20, 0x00fe53, 0x00fe54, 
	0x00fe67, 0x00fe68, 0x00fe6c, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff01, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 
	0x00ffdd, 0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fffc, 0x00fffe, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 
	0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x01018f, 0x010190, 0x01019d, 0x0101a0, 0x0101a1, 0x0101d0, 
	0x0101fe, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x0102e0, 0x0102fc, 0x010300, 0x010324, 0x01032d, 0x01034b, 0x010350, 0x01037b, 0x010380, 0x01039e, 0x01039f, 
	0x0103c4, 0x0103c8, 0x0103d6, 0x010400, 0x01049e, 0x0104a0, 0x0104aa, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 0x01056f, 
	0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010600, 
	0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 
	0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010857, 0x01089f, 0x0108a7, 0x0108b0, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x0108fb, 
	0x01091c, 0x01091f, 0x01093a, 0x01093f, 0x010940, 0x010980, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 
	0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a49, 0x010a50, 0x010a59, 0x010a60, 0x010aa0, 0x010ac0, 0x010ae7, 0x010aeb, 0x010af7, 0x010b00, 
	0x010b36, 0x010b39, 0x010b56, 0x010b58, 0x010b73, 0x010b78, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 
	0x010cf3, 0x010cfa, 0x010d28, 0x010d30, 0x010d3a, 0x010e60, 0x010e7f, 0x010e80, 0x010eaa, 0x010eab, 0x010eae, 0x010eb0, 0x010eb2, 0x010efd, 0x010f28, 0x010f30, 
	0x010f5a, 0x010f70, 0x010f8a, 0x010fb0, 0x010fcc, 0x010fe0, 0x010ff7, 0x011000, 0x01104e, 0x011052, 0x011076, 0x01107f, 0x0110bd, 0x0110be, 0x0110c3, 0x0110d0, 
	0x0110e9, 0x0110f0, 0x0110fa, 0x011100, 0x011135, 0x011136, 0x011148, 0x011150, 0x011177, 0x011180, 0x0111e0, 0x0111e1, 0x0111f5, 0x011200, 0x011212, 0x011213, 
	0x011242, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 0x011300, 
	0x011304, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133b, 0x011345, 0x011347, 
	0x011349, 0x01134b, 0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011400, 0x01145c, 0x01145d, 
	0x011462, 0x011480, 0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115b6, 0x0115b8, 0x0115de, 0x011600, 0x011645, 0x011650, 0x01165a, 0x011660, 0x01166d, 0x011680, 
	0x0116ba, 0x0116c0, 0x0116ca, 0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x011747, 0x011800, 0x01183c, 0x0118a0, 0x0118f3, 0x0118ff, 0x011907, 0x011909, 
	0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011947, 0x011950, 0x01195a, 0x0119a0, 0x0119a8, 0x0119aa, 
	0x0119d8, 0x0119da, 0x0119e5, 0x011a00, 0x011a48, 0x011a50, 0x011aa3, 0x011ab0, 0x011af9, 0x011b00, 0x011b0a, 0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 
	0x011c46, 0x011c50, 0x011c6d, 0x011c70, 0x011c90, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 
	0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 
	0x011d99, 0x011da0, 0x011daa, 0x011ee0, 0x011ef9, 0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f5a, 0x011fb0, 0x011fb1, 0x011fc0, 0x011ff2, 0x011fff, 
	0x01239a, 0x012400, 0x01246f, 0x012470, 0x012475, 0x012480, 0x012544, 0x012f90, 0x012ff3, 0x013000, 0x013430, 0x013440, 0x013456, 0x014400, 0x014647, 0x016800, 
	0x016a39, 0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a6e, 0x016abf, 0x016ac0, 0x016aca, 0x016ad0, 0x016aee, 0x016af0, 0x016af6, 0x016b00, 0x016b46, 0x016b50, 
	0x016b5a, 0x016b5b, 0x016b62, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e9b, 0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 0x016fe0, 
	0x016fe5, 0x016ff0, 0x016ff2, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 
	0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 
	0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9c, 0x01bca0, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 
	0x01d173, 0x01d17b, 0x01d1eb, 0x01d200, 0x01d246, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d300, 0x01d357, 0x01d360, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 
	0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 
	0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 
	0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01da8c, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 
	0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e130, 
	0x01e13e, 0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 0x01e290, 0x01e2af, 0x01e2c0, 0x01e2fa, 0x01e2ff, 0x01e300, 0x01e4d0, 0x01e4fa, 0x01e7e0, 0x01e7e7, 0x01e7e8, 
	0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01e8c7, 0x01e8d7, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 0x01ec71, 
	0x01ecb5, 0x01ed01, 0x01ed3e, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 
	0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 
	0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 
	0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 
	0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 0x01f0c1, 
	0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 
	0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 0x01f810, 
	0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 0x01fa80, 
	0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 0x01fbf0, 
	0x01fbfa, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 
	0x0323b0, 0x0e0100, 0x0e01f0, 0x110000, 
};
#define mxCharSet_General_Category_Other_Letter 1020
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Other_Letter[mxCharSet_General_Category_Other_Letter] = {
	0x0000aa, 0x0000ab, 0x0000ba, 0x0000bb, 0x0001bb, 0x0001bc, 0x0001c0, 0x0001c4, 0x000294, 0x000295, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f3, 0x000620, 0x000640, 
	0x000641, 0x00064b, 0x00066e, 0x000670, 0x000671, 0x0006d4, 0x0006d5, 0x0006d6, 0x0006ee, 0x0006f0, 0x0006fa, 0x0006fd, 0x0006ff, 0x000700, 0x000710, 0x000711, 
	0x000712, 0x000730, 0x00074d, 0x0007a6, 0x0007b1, 0x0007b2, 0x0007ca, 0x0007eb, 0x000800, 0x000816, 0x000840, 0x000859, 0x000860, 0x00086b, 0x000870, 0x000888, 
	0x000889, 0x00088f, 0x0008a0, 0x0008c9, 0x000904, 0x00093a, 0x00093d, 0x00093e, 0x000950, 0x000951, 0x000958, 0x000962, 0x000972, 0x000981, 0x000985, 0x00098d, 
	0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bd, 0x0009be, 0x0009ce, 0x0009cf, 0x0009dc, 0x0009de, 
	0x0009df, 0x0009e2, 0x0009f0, 0x0009f2, 0x0009fc, 0x0009fd, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 
	0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a72, 0x000a75, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 
	0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abd, 0x000abe, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae2, 0x000af9, 0x000afa, 0x000b05, 0x000b0d, 
	0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3d, 0x000b3e, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b62, 
	0x000b71, 0x000b72, 0x000b83, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 
	0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bd0, 0x000bd1, 0x000c05, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 
	0x000c3d, 0x000c3e, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c62, 0x000c80, 0x000c81, 0x000c85, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 
	0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbd, 0x000cbe, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce2, 0x000cf1, 0x000cf3, 0x000d04, 0x000d0d, 0x000d0e, 0x000d11, 
	0x000d12, 0x000d3b, 0x000d3d, 0x000d3e, 0x000d4e, 0x000d4f, 0x000d54, 0x000d57, 0x000d5f, 0x000d62, 0x000d7a, 0x000d80, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 
	0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000e01, 0x000e31, 0x000e32, 0x000e34, 0x000e40, 0x000e46, 0x000e81, 0x000e83, 0x000e84, 0x000e85, 
	0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000eb1, 0x000eb2, 0x000eb4, 0x000ebd, 0x000ebe, 0x000ec0, 0x000ec5, 0x000edc, 0x000ee0, 
	0x000f00, 0x000f01, 0x000f40, 0x000f48, 0x000f49, 0x000f6d, 0x000f88, 0x000f8d, 0x001000, 0x00102b, 0x00103f, 0x001040, 0x001050, 0x001056, 0x00105a, 0x00105e, 
	0x001061, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001071, 0x001075, 0x001082, 0x00108e, 0x00108f, 0x001100, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 
	0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 
	0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x001380, 0x001390, 0x001401, 0x00166d, 0x00166f, 0x001680, 
	0x001681, 0x00169b, 0x0016a0, 0x0016eb, 0x0016f1, 0x0016f9, 0x001700, 0x001712, 0x00171f, 0x001732, 0x001740, 0x001752, 0x001760, 0x00176d, 0x00176e, 0x001771, 
	0x001780, 0x0017b4, 0x0017dc, 0x0017dd, 0x001820, 0x001843, 0x001844, 0x001879, 0x001880, 0x001885, 0x001887, 0x0018a9, 0x0018aa, 0x0018ab, 0x0018b0, 0x0018f6, 
	0x001900, 0x00191f, 0x001950, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x001a00, 0x001a17, 0x001a20, 0x001a55, 0x001b05, 0x001b34, 
	0x001b45, 0x001b4d, 0x001b83, 0x001ba1, 0x001bae, 0x001bb0, 0x001bba, 0x001be6, 0x001c00, 0x001c24, 0x001c4d, 0x001c50, 0x001c5a, 0x001c78, 0x001ce9, 0x001ced, 
	0x001cee, 0x001cf4, 0x001cf5, 0x001cf7, 0x001cfa, 0x001cfb, 0x002135, 0x002139, 0x002d30, 0x002d68, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 
	0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x003006, 0x003007, 0x00303c, 0x00303d, 
	0x003041, 0x003097, 0x00309f, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030ff, 0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x0031a0, 0x0031c0, 0x0031f0, 0x003200, 
	0x003400, 0x004dc0, 0x004e00, 0x00a015, 0x00a016, 0x00a48d, 0x00a4d0, 0x00a4f8, 0x00a500, 0x00a60c, 0x00a610, 0x00a620, 0x00a62a, 0x00a62c, 0x00a66e, 0x00a66f, 
	0x00a6a0, 0x00a6e6, 0x00a78f, 0x00a790, 0x00a7f7, 0x00a7f8, 0x00a7fb, 0x00a802, 0x00a803, 0x00a806, 0x00a807, 0x00a80b, 0x00a80c, 0x00a823, 0x00a840, 0x00a874, 
	0x00a882, 0x00a8b4, 0x00a8f2, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a8ff, 0x00a90a, 0x00a926, 0x00a930, 0x00a947, 0x00a960, 0x00a97d, 0x00a984, 0x00a9b3, 
	0x00a9e0, 0x00a9e5, 0x00a9e7, 0x00a9f0, 0x00a9fa, 0x00a9ff, 0x00aa00, 0x00aa29, 0x00aa40, 0x00aa43, 0x00aa44, 0x00aa4c, 0x00aa60, 0x00aa70, 0x00aa71, 0x00aa77, 
	0x00aa7a, 0x00aa7b, 0x00aa7e, 0x00aab0, 0x00aab1, 0x00aab2, 0x00aab5, 0x00aab7, 0x00aab9, 0x00aabe, 0x00aac0, 0x00aac1, 0x00aac2, 0x00aac3, 0x00aadb, 0x00aadd, 
	0x00aae0, 0x00aaeb, 0x00aaf2, 0x00aaf3, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 0x00abc0, 0x00abe3, 
	0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x00fb1d, 0x00fb1e, 0x00fb1f, 0x00fb29, 0x00fb2a, 0x00fb37, 
	0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 0x00fbb2, 0x00fbd3, 0x00fd3e, 0x00fd50, 0x00fd90, 0x00fd92, 0x00fdc8, 
	0x00fdf0, 0x00fdfc, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x00ff66, 0x00ff70, 0x00ff71, 0x00ff9e, 0x00ffa0, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 
	0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 
	0x010080, 0x0100fb, 0x010280, 0x01029d, 0x0102a0, 0x0102d1, 0x010300, 0x010320, 0x01032d, 0x010341, 0x010342, 0x01034a, 0x010350, 0x010376, 0x010380, 0x01039e, 
	0x0103a0, 0x0103c4, 0x0103c8, 0x0103d0, 0x010450, 0x01049e, 0x010500, 0x010528, 0x010530, 0x010564, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 
	0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010860, 0x010877, 0x010880, 0x01089f, 
	0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x010900, 0x010916, 0x010920, 0x01093a, 0x010980, 0x0109b8, 0x0109be, 0x0109c0, 0x010a00, 0x010a01, 0x010a10, 0x010a14, 
	0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a60, 0x010a7d, 0x010a80, 0x010a9d, 0x010ac0, 0x010ac8, 0x010ac9, 0x010ae5, 0x010b00, 0x010b36, 0x010b40, 0x010b56, 
	0x010b60, 0x010b73, 0x010b80, 0x010b92, 0x010c00, 0x010c49, 0x010d00, 0x010d24, 0x010e80, 0x010eaa, 0x010eb0, 0x010eb2, 0x010f00, 0x010f1d, 0x010f27, 0x010f28, 
	0x010f30, 0x010f46, 0x010f70, 0x010f82, 0x010fb0, 0x010fc5, 0x010fe0, 0x010ff7, 0x011003, 0x011038, 0x011071, 0x011073, 0x011075, 0x011076, 0x011083, 0x0110b0, 
	0x0110d0, 0x0110e9, 0x011103, 0x011127, 0x011144, 0x011145, 0x011147, 0x011148, 0x011150, 0x011173, 0x011176, 0x011177, 0x011183, 0x0111b3, 0x0111c1, 0x0111c5, 
	0x0111da, 0x0111db, 0x0111dc, 0x0111dd, 0x011200, 0x011212, 0x011213, 0x01122c, 0x01123f, 0x011241, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 
	0x01128f, 0x01129e, 0x01129f, 0x0112a9, 0x0112b0, 0x0112df, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 
	0x011335, 0x01133a, 0x01133d, 0x01133e, 0x011350, 0x011351, 0x01135d, 0x011362, 0x011400, 0x011435, 0x011447, 0x01144b, 0x01145f, 0x011462, 0x011480, 0x0114b0, 
	0x0114c4, 0x0114c6, 0x0114c7, 0x0114c8, 0x011580, 0x0115af, 0x0115d8, 0x0115dc, 0x011600, 0x011630, 0x011644, 0x011645, 0x011680, 0x0116ab, 0x0116b8, 0x0116b9, 
	0x011700, 0x01171b, 0x011740, 0x011747, 0x011800, 0x01182c, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011930, 
	0x01193f, 0x011940, 0x011941, 0x011942, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d1, 0x0119e1, 0x0119e2, 0x0119e3, 0x0119e4, 0x011a00, 0x011a01, 0x011a0b, 0x011a33, 
	0x011a3a, 0x011a3b, 0x011a50, 0x011a51, 0x011a5c, 0x011a8a, 0x011a9d, 0x011a9e, 0x011ab0, 0x011af9, 0x011c00, 0x011c09, 0x011c0a, 0x011c2f, 0x011c40, 0x011c41, 
	0x011c72, 0x011c90, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d31, 0x011d46, 0x011d47, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8a, 
	0x011d98, 0x011d99, 0x011ee0, 0x011ef3, 0x011f02, 0x011f03, 0x011f04, 0x011f11, 0x011f12, 0x011f34, 0x011fb0, 0x011fb1, 0x012000, 0x01239a, 0x012480, 0x012544, 
	0x012f90, 0x012ff1, 0x013000, 0x013430, 0x013441, 0x013447, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a70, 0x016abf, 0x016ad0, 0x016aee, 
	0x016b00, 0x016b30, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 0x016f00, 0x016f4b, 0x016f50, 0x016f51, 0x017000, 0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 
	0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 
	0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01df0a, 0x01df0b, 0x01e100, 0x01e12d, 0x01e14e, 0x01e14f, 0x01e290, 0x01e2ae, 0x01e2c0, 0x01e2ec, 0x01e4d0, 0x01e4eb, 
	0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 0x01e8c5, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 
	0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 
	0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 
	0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 
	0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 
	0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_General_Category_Other_Number 144
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Other_Number[mxCharSet_General_Category_Other_Number] = {
	0x0000b2, 0x0000b4, 0x0000b9, 0x0000ba, 0x0000bc, 0x0000bf, 0x0009f4, 0x0009fa, 0x000b72, 0x000b78, 0x000bf0, 0x000bf3, 0x000c78, 0x000c7f, 0x000d58, 0x000d5f, 
	0x000d70, 0x000d79, 0x000f2a, 0x000f34, 0x001369, 0x00137d, 0x0017f0, 0x0017fa, 0x0019da, 0x0019db, 0x002070, 0x002071, 0x002074, 0x00207a, 0x002080, 0x00208a, 
	0x002150, 0x002160, 0x002189, 0x00218a, 0x002460, 0x00249c, 0x0024ea, 0x002500, 0x002776, 0x002794, 0x002cfd, 0x002cfe, 0x003192, 0x003196, 0x003220, 0x00322a, 
	0x003248, 0x003250, 0x003251, 0x003260, 0x003280, 0x00328a, 0x0032b1, 0x0032c0, 0x00a830, 0x00a836, 0x010107, 0x010134, 0x010175, 0x010179, 0x01018a, 0x01018c, 
	0x0102e1, 0x0102fc, 0x010320, 0x010324, 0x010858, 0x010860, 0x010879, 0x010880, 0x0108a7, 0x0108b0, 0x0108fb, 0x010900, 0x010916, 0x01091c, 0x0109bc, 0x0109be, 
	0x0109c0, 0x0109d0, 0x0109d2, 0x010a00, 0x010a40, 0x010a49, 0x010a7d, 0x010a7f, 0x010a9d, 0x010aa0, 0x010aeb, 0x010af0, 0x010b58, 0x010b60, 0x010b78, 0x010b80, 
	0x010ba9, 0x010bb0, 0x010cfa, 0x010d00, 0x010e60, 0x010e7f, 0x010f1d, 0x010f27, 0x010f51, 0x010f55, 0x010fc5, 0x010fcc, 0x011052, 0x011066, 0x0111e1, 0x0111f5, 
	0x01173a, 0x01173c, 0x0118ea, 0x0118f3, 0x011c5a, 0x011c6d, 0x011fc0, 0x011fd5, 0x016b5b, 0x016b62, 0x016e80, 0x016e97, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 
	0x01d360, 0x01d379, 0x01e8c7, 0x01e8d0, 0x01ec71, 0x01ecac, 0x01ecad, 0x01ecb0, 0x01ecb1, 0x01ecb5, 0x01ed01, 0x01ed2e, 0x01ed2f, 0x01ed3e, 0x01f100, 0x01f10d, 
};
#define mxCharSet_General_Category_Other_Punctuation 374
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Other_Punctuation[mxCharSet_General_Category_Other_Punctuation] = {
	0x000021, 0x000024, 0x000025, 0x000028, 0x00002a, 0x00002b, 0x00002c, 0x00002d, 0x00002e, 0x000030, 0x00003a, 0x00003c, 0x00003f, 0x000041, 0x00005c, 0x00005d, 
	0x0000a1, 0x0000a2, 0x0000a7, 0x0000a8, 0x0000b6, 0x0000b8, 0x0000bf, 0x0000c0, 0x00037e, 0x00037f, 0x000387, 0x000388, 0x00055a, 0x000560, 0x000589, 0x00058a, 
	0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005f3, 0x0005f5, 0x000609, 0x00060b, 0x00060c, 0x00060e, 0x00061b, 0x00061c, 0x00061d, 0x000620, 
	0x00066a, 0x00066e, 0x0006d4, 0x0006d5, 0x000700, 0x00070e, 0x0007f7, 0x0007fa, 0x000830, 0x00083f, 0x00085e, 0x00085f, 0x000964, 0x000966, 0x000970, 0x000971, 
	0x0009fd, 0x0009fe, 0x000a76, 0x000a77, 0x000af0, 0x000af1, 0x000c77, 0x000c78, 0x000c84, 0x000c85, 0x000df4, 0x000df5, 0x000e4f, 0x000e50, 0x000e5a, 0x000e5c, 
	0x000f04, 0x000f13, 0x000f14, 0x000f15, 0x000f85, 0x000f86, 0x000fd0, 0x000fd5, 0x000fd9, 0x000fdb, 0x00104a, 0x001050, 0x0010fb, 0x0010fc, 0x001360, 0x001369, 
	0x00166e, 0x00166f, 0x0016eb, 0x0016ee, 0x001735, 0x001737, 0x0017d4, 0x0017d7, 0x0017d8, 0x0017db, 0x001800, 0x001806, 0x001807, 0x00180b, 0x001944, 0x001946, 
	0x001a1e, 0x001a20, 0x001aa0, 0x001aa7, 0x001aa8, 0x001aae, 0x001b5a, 0x001b61, 0x001b7d, 0x001b7f, 0x001bfc, 0x001c00, 0x001c3b, 0x001c40, 0x001c7e, 0x001c80, 
	0x001cc0, 0x001cc8, 0x001cd3, 0x001cd4, 0x002016, 0x002018, 0x002020, 0x002028, 0x002030, 0x002039, 0x00203b, 0x00203f, 0x002041, 0x002044, 0x002047, 0x002052, 
	0x002053, 0x002054, 0x002055, 0x00205f, 0x002cf9, 0x002cfd, 0x002cfe, 0x002d00, 0x002d70, 0x002d71, 0x002e00, 0x002e02, 0x002e06, 0x002e09, 0x002e0b, 0x002e0c, 
	0x002e0e, 0x002e17, 0x002e18, 0x002e1a, 0x002e1b, 0x002e1c, 0x002e1e, 0x002e20, 0x002e2a, 0x002e2f, 0x002e30, 0x002e3a, 0x002e3c, 0x002e40, 0x002e41, 0x002e42, 
	0x002e43, 0x002e50, 0x002e52, 0x002e55, 0x003001, 0x003004, 0x00303d, 0x00303e, 0x0030fb, 0x0030fc, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a673, 0x00a674, 
	0x00a67e, 0x00a67f, 0x00a6f2, 0x00a6f8, 0x00a874, 0x00a878, 0x00a8ce, 0x00a8d0, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a92e, 0x00a930, 0x00a95f, 0x00a960, 
	0x00a9c1, 0x00a9ce, 0x00a9de, 0x00a9e0, 0x00aa5c, 0x00aa60, 0x00aade, 0x00aae0, 0x00aaf0, 0x00aaf2, 0x00abeb, 0x00abec, 0x00fe10, 0x00fe17, 0x00fe19, 0x00fe1a, 
	0x00fe30, 0x00fe31, 0x00fe45, 0x00fe47, 0x00fe49, 0x00fe4d, 0x00fe50, 0x00fe53, 0x00fe54, 0x00fe58, 0x00fe5f, 0x00fe62, 0x00fe68, 0x00fe69, 0x00fe6a, 0x00fe6c, 
	0x00ff01, 0x00ff04, 0x00ff05, 0x00ff08, 0x00ff0a, 0x00ff0b, 0x00ff0c, 0x00ff0d, 0x00ff0e, 0x00ff10, 0x00ff1a, 0x00ff1c, 0x00ff1f, 0x00ff21, 0x00ff3c, 0x00ff3d, 
	0x00ff61, 0x00ff62, 0x00ff64, 0x00ff66, 0x010100, 0x010103, 0x01039f, 0x0103a0, 0x0103d0, 0x0103d1, 0x01056f, 0x010570, 0x010857, 0x010858, 0x01091f, 0x010920, 
	0x01093f, 0x010940, 0x010a50, 0x010a59, 0x010a7f, 0x010a80, 0x010af0, 0x010af7, 0x010b39, 0x010b40, 0x010b99, 0x010b9d, 0x010f55, 0x010f5a, 0x010f86, 0x010f8a, 
	0x011047, 0x01104e, 0x0110bb, 0x0110bd, 0x0110be, 0x0110c2, 0x011140, 0x011144, 0x011174, 0x011176, 0x0111c5, 0x0111c9, 0x0111cd, 0x0111ce, 0x0111db, 0x0111dc, 
	0x0111dd, 0x0111e0, 0x011238, 0x01123e, 0x0112a9, 0x0112aa, 0x01144b, 0x011450, 0x01145a, 0x01145c, 0x01145d, 0x01145e, 0x0114c6, 0x0114c7, 0x0115c1, 0x0115d8, 
	0x011641, 0x011644, 0x011660, 0x01166d, 0x0116b9, 0x0116ba, 0x01173c, 0x01173f, 0x01183b, 0x01183c, 0x011944, 0x011947, 0x0119e2, 0x0119e3, 0x011a3f, 0x011a47, 
	0x011a9a, 0x011a9d, 0x011a9e, 0x011aa3, 0x011b00, 0x011b0a, 0x011c41, 0x011c46, 0x011c70, 0x011c72, 0x011ef7, 0x011ef9, 0x011f43, 0x011f50, 0x011fff, 0x012000, 
	0x012470, 0x012475, 0x012ff1, 0x012ff3, 0x016a6e, 0x016a70, 0x016af5, 0x016af6, 0x016b37, 0x016b3c, 0x016b44, 0x016b45, 0x016e97, 0x016e9b, 0x016fe2, 0x016fe3, 
	0x01bc9f, 0x01bca0, 0x01da87, 0x01da8c, 0x01e95e, 0x01e960, 
};
#define mxCharSet_General_Category_Other_Symbol 368
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Other_Symbol[mxCharSet_General_Category_Other_Symbol] = {
	0x0000a6, 0x0000a7, 0x0000a9, 0x0000aa, 0x0000ae, 0x0000af, 0x0000b0, 0x0000b1, 0x000482, 0x000483, 0x00058d, 0x00058f, 0x00060e, 0x000610, 0x0006de, 0x0006df, 
	0x0006e9, 0x0006ea, 0x0006fd, 0x0006ff, 0x0007f6, 0x0007f7, 0x0009fa, 0x0009fb, 0x000b70, 0x000b71, 0x000bf3, 0x000bf9, 0x000bfa, 0x000bfb, 0x000c7f, 0x000c80, 
	0x000d4f, 0x000d50, 0x000d79, 0x000d7a, 0x000f01, 0x000f04, 0x000f13, 0x000f14, 0x000f15, 0x000f18, 0x000f1a, 0x000f20, 0x000f34, 0x000f35, 0x000f36, 0x000f37, 
	0x000f38, 0x000f39, 0x000fbe, 0x000fc6, 0x000fc7, 0x000fcd, 0x000fce, 0x000fd0, 0x000fd5, 0x000fd9, 0x00109e, 0x0010a0, 0x001390, 0x00139a, 0x00166d, 0x00166e, 
	0x001940, 0x001941, 0x0019de, 0x001a00, 0x001b61, 0x001b6b, 0x001b74, 0x001b7d, 0x002100, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 
	0x002116, 0x002118, 0x00211e, 0x002124, 0x002125, 0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x00213a, 0x00213c, 0x00214a, 0x00214b, 
	0x00214c, 0x00214e, 0x00214f, 0x002150, 0x00218a, 0x00218c, 0x002195, 0x00219a, 0x00219c, 0x0021a0, 0x0021a1, 0x0021a3, 0x0021a4, 0x0021a6, 0x0021a7, 0x0021ae, 
	0x0021af, 0x0021ce, 0x0021d0, 0x0021d2, 0x0021d3, 0x0021d4, 0x0021d5, 0x0021f4, 0x002300, 0x002308, 0x00230c, 0x002320, 0x002322, 0x002329, 0x00232b, 0x00237c, 
	0x00237d, 0x00239b, 0x0023b4, 0x0023dc, 0x0023e2, 0x002427, 0x002440, 0x00244b, 0x00249c, 0x0024ea, 0x002500, 0x0025b7, 0x0025b8, 0x0025c1, 0x0025c2, 0x0025f8, 
	0x002600, 0x00266f, 0x002670, 0x002768, 0x002794, 0x0027c0, 0x002800, 0x002900, 0x002b00, 0x002b30, 0x002b45, 0x002b47, 0x002b4d, 0x002b74, 0x002b76, 0x002b96, 
	0x002b97, 0x002c00, 0x002ce5, 0x002ceb, 0x002e50, 0x002e52, 0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x002ff0, 0x002ffc, 0x003004, 0x003005, 
	0x003012, 0x003014, 0x003020, 0x003021, 0x003036, 0x003038, 0x00303e, 0x003040, 0x003190, 0x003192, 0x003196, 0x0031a0, 0x0031c0, 0x0031e4, 0x003200, 0x00321f, 
	0x00322a, 0x003248, 0x003250, 0x003251, 0x003260, 0x003280, 0x00328a, 0x0032b1, 0x0032c0, 0x003400, 0x004dc0, 0x004e00, 0x00a490, 0x00a4c7, 0x00a828, 0x00a82c, 
	0x00a836, 0x00a838, 0x00a839, 0x00a83a, 0x00aa77, 0x00aa7a, 0x00fd40, 0x00fd50, 0x00fdcf, 0x00fdd0, 0x00fdfd, 0x00fe00, 0x00ffe4, 0x00ffe5, 0x00ffe8, 0x00ffe9, 
	0x00ffed, 0x00ffef, 0x00fffc, 0x00fffe, 0x010137, 0x010140, 0x010179, 0x01018a, 0x01018c, 0x01018f, 0x010190, 0x01019d, 0x0101a0, 0x0101a1, 0x0101d0, 0x0101fd, 
	0x010877, 0x010879, 0x010ac8, 0x010ac9, 0x01173f, 0x011740, 0x011fd5, 0x011fdd, 0x011fe1, 0x011ff2, 0x016b3c, 0x016b40, 0x016b45, 0x016b46, 0x01bc9c, 0x01bc9d, 
	0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d165, 0x01d16a, 0x01d16d, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d1eb, 
	0x01d200, 0x01d242, 0x01d245, 0x01d246, 0x01d300, 0x01d357, 0x01d800, 0x01da00, 0x01da37, 0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da87, 
	0x01e14f, 0x01e150, 0x01ecac, 0x01ecad, 0x01ed2e, 0x01ed2f, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 0x01f0c1, 0x01f0d0, 
	0x01f0d1, 0x01f0f6, 0x01f10d, 0x01f1ae, 0x01f1e6, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 0x01f3fb, 
	0x01f400, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 
	0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 
	0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 
};
#define mxCharSet_General_Category_Paragraph_Separator 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Paragraph_Separator[mxCharSet_General_Category_Paragraph_Separator] = {
	0x002029, 0x00202a, 
};
#define mxCharSet_General_Category_Private_Use 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Private_Use[mxCharSet_General_Category_Private_Use] = {
	0x00e000, 0x00f900, 0x0f0000, 0x0ffffe, 0x100000, 0x10fffe, 
};
#define mxCharSet_General_Category_Punctuation 382
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Punctuation[mxCharSet_General_Category_Punctuation] = {
	0x000021, 0x000024, 0x000025, 0x00002b, 0x00002c, 0x000030, 0x00003a, 0x00003c, 0x00003f, 0x000041, 0x00005b, 0x00005e, 0x00005f, 0x000060, 0x00007b, 0x00007c, 
	0x00007d, 0x00007e, 0x0000a1, 0x0000a2, 0x0000a7, 0x0000a8, 0x0000ab, 0x0000ac, 0x0000b6, 0x0000b8, 0x0000bb, 0x0000bc, 0x0000bf, 0x0000c0, 0x00037e, 0x00037f, 
	0x000387, 0x000388, 0x00055a, 0x000560, 0x000589, 0x00058b, 0x0005be, 0x0005bf, 0x0005c0, 0x0005c1, 0x0005c3, 0x0005c4, 0x0005c6, 0x0005c7, 0x0005f3, 0x0005f5, 
	0x000609, 0x00060b, 0x00060c, 0x00060e, 0x00061b, 0x00061c, 0x00061d, 0x000620, 0x00066a, 0x00066e, 0x0006d4, 0x0006d5, 0x000700, 0x00070e, 0x0007f7, 0x0007fa, 
	0x000830, 0x00083f, 0x00085e, 0x00085f, 0x000964, 0x000966, 0x000970, 0x000971, 0x0009fd, 0x0009fe, 0x000a76, 0x000a77, 0x000af0, 0x000af1, 0x000c77, 0x000c78, 
	0x000c84, 0x000c85, 0x000df4, 0x000df5, 0x000e4f, 0x000e50, 0x000e5a, 0x000e5c, 0x000f04, 0x000f13, 0x000f14, 0x000f15, 0x000f3a, 0x000f3e, 0x000f85, 0x000f86, 
	0x000fd0, 0x000fd5, 0x000fd9, 0x000fdb, 0x00104a, 0x001050, 0x0010fb, 0x0010fc, 0x001360, 0x001369, 0x001400, 0x001401, 0x00166e, 0x00166f, 0x00169b, 0x00169d, 
	0x0016eb, 0x0016ee, 0x001735, 0x001737, 0x0017d4, 0x0017d7, 0x0017d8, 0x0017db, 0x001800, 0x00180b, 0x001944, 0x001946, 0x001a1e, 0x001a20, 0x001aa0, 0x001aa7, 
	0x001aa8, 0x001aae, 0x001b5a, 0x001b61, 0x001b7d, 0x001b7f, 0x001bfc, 0x001c00, 0x001c3b, 0x001c40, 0x001c7e, 0x001c80, 0x001cc0, 0x001cc8, 0x001cd3, 0x001cd4, 
	0x002010, 0x002028, 0x002030, 0x002044, 0x002045, 0x002052, 0x002053, 0x00205f, 0x00207d, 0x00207f, 0x00208d, 0x00208f, 0x002308, 0x00230c, 0x002329, 0x00232b, 
	0x002768, 0x002776, 0x0027c5, 0x0027c7, 0x0027e6, 0x0027f0, 0x002983, 0x002999, 0x0029d8, 0x0029dc, 0x0029fc, 0x0029fe, 0x002cf9, 0x002cfd, 0x002cfe, 0x002d00, 
	0x002d70, 0x002d71, 0x002e00, 0x002e2f, 0x002e30, 0x002e50, 0x002e52, 0x002e5e, 0x003001, 0x003004, 0x003008, 0x003012, 0x003014, 0x003020, 0x003030, 0x003031, 
	0x00303d, 0x00303e, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fc, 0x00a4fe, 0x00a500, 0x00a60d, 0x00a610, 0x00a673, 0x00a674, 0x00a67e, 0x00a67f, 0x00a6f2, 0x00a6f8, 
	0x00a874, 0x00a878, 0x00a8ce, 0x00a8d0, 0x00a8f8, 0x00a8fb, 0x00a8fc, 0x00a8fd, 0x00a92e, 0x00a930, 0x00a95f, 0x00a960, 0x00a9c1, 0x00a9ce, 0x00a9de, 0x00a9e0, 
	0x00aa5c, 0x00aa60, 0x00aade, 0x00aae0, 0x00aaf0, 0x00aaf2, 0x00abeb, 0x00abec, 0x00fd3e, 0x00fd40, 0x00fe10, 0x00fe1a, 0x00fe30, 0x00fe53, 0x00fe54, 0x00fe62, 
	0x00fe63, 0x00fe64, 0x00fe68, 0x00fe69, 0x00fe6a, 0x00fe6c, 0x00ff01, 0x00ff04, 0x00ff05, 0x00ff0b, 0x00ff0c, 0x00ff10, 0x00ff1a, 0x00ff1c, 0x00ff1f, 0x00ff21, 
	0x00ff3b, 0x00ff3e, 0x00ff3f, 0x00ff40, 0x00ff5b, 0x00ff5c, 0x00ff5d, 0x00ff5e, 0x00ff5f, 0x00ff66, 0x010100, 0x010103, 0x01039f, 0x0103a0, 0x0103d0, 0x0103d1, 
	0x01056f, 0x010570, 0x010857, 0x010858, 0x01091f, 0x010920, 0x01093f, 0x010940, 0x010a50, 0x010a59, 0x010a7f, 0x010a80, 0x010af0, 0x010af7, 0x010b39, 0x010b40, 
	0x010b99, 0x010b9d, 0x010ead, 0x010eae, 0x010f55, 0x010f5a, 0x010f86, 0x010f8a, 0x011047, 0x01104e, 0x0110bb, 0x0110bd, 0x0110be, 0x0110c2, 0x011140, 0x011144, 
	0x011174, 0x011176, 0x0111c5, 0x0111c9, 0x0111cd, 0x0111ce, 0x0111db, 0x0111dc, 0x0111dd, 0x0111e0, 0x011238, 0x01123e, 0x0112a9, 0x0112aa, 0x01144b, 0x011450, 
	0x01145a, 0x01145c, 0x01145d, 0x01145e, 0x0114c6, 0x0114c7, 0x0115c1, 0x0115d8, 0x011641, 0x011644, 0x011660, 0x01166d, 0x0116b9, 0x0116ba, 0x01173c, 0x01173f, 
	0x01183b, 0x01183c, 0x011944, 0x011947, 0x0119e2, 0x0119e3, 0x011a3f, 0x011a47, 0x011a9a, 0x011a9d, 0x011a9e, 0x011aa3, 0x011b00, 0x011b0a, 0x011c41, 0x011c46, 
	0x011c70, 0x011c72, 0x011ef7, 0x011ef9, 0x011f43, 0x011f50, 0x011fff, 0x012000, 0x012470, 0x012475, 0x012ff1, 0x012ff3, 0x016a6e, 0x016a70, 0x016af5, 0x016af6, 
	0x016b37, 0x016b3c, 0x016b44, 0x016b45, 0x016e97, 0x016e9b, 0x016fe2, 0x016fe3, 0x01bc9f, 0x01bca0, 0x01da87, 0x01da8c, 0x01e95e, 0x01e960, 
};
#define mxCharSet_General_Category_Separator 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Separator[mxCharSet_General_Category_Separator] = {
	0x000020, 0x000021, 0x0000a0, 0x0000a1, 0x001680, 0x001681, 0x002000, 0x00200b, 0x002028, 0x00202a, 0x00202f, 0x002030, 0x00205f, 0x002060, 0x003000, 0x003001, 
};
#define mxCharSet_General_Category_Space_Separator 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Space_Separator[mxCharSet_General_Category_Space_Separator] = {
	0x000020, 0x000021, 0x0000a0, 0x0000a1, 0x001680, 0x001681, 0x002000, 0x00200b, 0x00202f, 0x002030, 0x00205f, 0x002060, 0x003000, 0x003001, 
};
#define mxCharSet_General_Category_Spacing_Mark 364
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Spacing_Mark[mxCharSet_General_Category_Spacing_Mark] = {
	0x000903, 0x000904, 0x00093b, 0x00093c, 0x00093e, 0x000941, 0x000949, 0x00094d, 0x00094e, 0x000950, 0x000982, 0x000984, 0x0009be, 0x0009c1, 0x0009c7, 0x0009c9, 
	0x0009cb, 0x0009cd, 0x0009d7, 0x0009d8, 0x000a03, 0x000a04, 0x000a3e, 0x000a41, 0x000a83, 0x000a84, 0x000abe, 0x000ac1, 0x000ac9, 0x000aca, 0x000acb, 0x000acd, 
	0x000b02, 0x000b04, 0x000b3e, 0x000b3f, 0x000b40, 0x000b41, 0x000b47, 0x000b49, 0x000b4b, 0x000b4d, 0x000b57, 0x000b58, 0x000bbe, 0x000bc0, 0x000bc1, 0x000bc3, 
	0x000bc6, 0x000bc9, 0x000bca, 0x000bcd, 0x000bd7, 0x000bd8, 0x000c01, 0x000c04, 0x000c41, 0x000c45, 0x000c82, 0x000c84, 0x000cbe, 0x000cbf, 0x000cc0, 0x000cc5, 
	0x000cc7, 0x000cc9, 0x000cca, 0x000ccc, 0x000cd5, 0x000cd7, 0x000cf3, 0x000cf4, 0x000d02, 0x000d04, 0x000d3e, 0x000d41, 0x000d46, 0x000d49, 0x000d4a, 0x000d4d, 
	0x000d57, 0x000d58, 0x000d82, 0x000d84, 0x000dcf, 0x000dd2, 0x000dd8, 0x000de0, 0x000df2, 0x000df4, 0x000f3e, 0x000f40, 0x000f7f, 0x000f80, 0x00102b, 0x00102d, 
	0x001031, 0x001032, 0x001038, 0x001039, 0x00103b, 0x00103d, 0x001056, 0x001058, 0x001062, 0x001065, 0x001067, 0x00106e, 0x001083, 0x001085, 0x001087, 0x00108d, 
	0x00108f, 0x001090, 0x00109a, 0x00109d, 0x001715, 0x001716, 0x001734, 0x001735, 0x0017b6, 0x0017b7, 0x0017be, 0x0017c6, 0x0017c7, 0x0017c9, 0x001923, 0x001927, 
	0x001929, 0x00192c, 0x001930, 0x001932, 0x001933, 0x001939, 0x001a19, 0x001a1b, 0x001a55, 0x001a56, 0x001a57, 0x001a58, 0x001a61, 0x001a62, 0x001a63, 0x001a65, 
	0x001a6d, 0x001a73, 0x001b04, 0x001b05, 0x001b35, 0x001b36, 0x001b3b, 0x001b3c, 0x001b3d, 0x001b42, 0x001b43, 0x001b45, 0x001b82, 0x001b83, 0x001ba1, 0x001ba2, 
	0x001ba6, 0x001ba8, 0x001baa, 0x001bab, 0x001be7, 0x001be8, 0x001bea, 0x001bed, 0x001bee, 0x001bef, 0x001bf2, 0x001bf4, 0x001c24, 0x001c2c, 0x001c34, 0x001c36, 
	0x001ce1, 0x001ce2, 0x001cf7, 0x001cf8, 0x00302e, 0x003030, 0x00a823, 0x00a825, 0x00a827, 0x00a828, 0x00a880, 0x00a882, 0x00a8b4, 0x00a8c4, 0x00a952, 0x00a954, 
	0x00a983, 0x00a984, 0x00a9b4, 0x00a9b6, 0x00a9ba, 0x00a9bc, 0x00a9be, 0x00a9c1, 0x00aa2f, 0x00aa31, 0x00aa33, 0x00aa35, 0x00aa4d, 0x00aa4e, 0x00aa7b, 0x00aa7c, 
	0x00aa7d, 0x00aa7e, 0x00aaeb, 0x00aaec, 0x00aaee, 0x00aaf0, 0x00aaf5, 0x00aaf6, 0x00abe3, 0x00abe5, 0x00abe6, 0x00abe8, 0x00abe9, 0x00abeb, 0x00abec, 0x00abed, 
	0x011000, 0x011001, 0x011002, 0x011003, 0x011082, 0x011083, 0x0110b0, 0x0110b3, 0x0110b7, 0x0110b9, 0x01112c, 0x01112d, 0x011145, 0x011147, 0x011182, 0x011183, 
	0x0111b3, 0x0111b6, 0x0111bf, 0x0111c1, 0x0111ce, 0x0111cf, 0x01122c, 0x01122f, 0x011232, 0x011234, 0x011235, 0x011236, 0x0112e0, 0x0112e3, 0x011302, 0x011304, 
	0x01133e, 0x011340, 0x011341, 0x011345, 0x011347, 0x011349, 0x01134b, 0x01134e, 0x011357, 0x011358, 0x011362, 0x011364, 0x011435, 0x011438, 0x011440, 0x011442, 
	0x011445, 0x011446, 0x0114b0, 0x0114b3, 0x0114b9, 0x0114ba, 0x0114bb, 0x0114bf, 0x0114c1, 0x0114c2, 0x0115af, 0x0115b2, 0x0115b8, 0x0115bc, 0x0115be, 0x0115bf, 
	0x011630, 0x011633, 0x01163b, 0x01163d, 0x01163e, 0x01163f, 0x0116ac, 0x0116ad, 0x0116ae, 0x0116b0, 0x0116b6, 0x0116b7, 0x011720, 0x011722, 0x011726, 0x011727, 
	0x01182c, 0x01182f, 0x011838, 0x011839, 0x011930, 0x011936, 0x011937, 0x011939, 0x01193d, 0x01193e, 0x011940, 0x011941, 0x011942, 0x011943, 0x0119d1, 0x0119d4, 
	0x0119dc, 0x0119e0, 0x0119e4, 0x0119e5, 0x011a39, 0x011a3a, 0x011a57, 0x011a59, 0x011a97, 0x011a98, 0x011c2f, 0x011c30, 0x011c3e, 0x011c3f, 0x011ca9, 0x011caa, 
	0x011cb1, 0x011cb2, 0x011cb4, 0x011cb5, 0x011d8a, 0x011d8f, 0x011d93, 0x011d95, 0x011d96, 0x011d97, 0x011ef5, 0x011ef7, 0x011f03, 0x011f04, 0x011f34, 0x011f36, 
	0x011f3e, 0x011f40, 0x011f41, 0x011f42, 0x016f51, 0x016f88, 0x016ff0, 0x016ff2, 0x01d165, 0x01d167, 0x01d16d, 0x01d173, 
};
#define mxCharSet_General_Category_Surrogate 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Surrogate[mxCharSet_General_Category_Surrogate] = {
	0x00d800, 0x00dc00, 0x00dc00, 0x00e000, 
};
#define mxCharSet_General_Category_Symbol 464
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Symbol[mxCharSet_General_Category_Symbol] = {
	0x000024, 0x000025, 0x00002b, 0x00002c, 0x00003c, 0x00003f, 0x00005e, 0x00005f, 0x000060, 0x000061, 0x00007c, 0x00007d, 0x00007e, 0x00007f, 0x0000a2, 0x0000a7, 
	0x0000a8, 0x0000aa, 0x0000ac, 0x0000ad, 0x0000ae, 0x0000b2, 0x0000b4, 0x0000b5, 0x0000b8, 0x0000b9, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002c2, 0x0002c6, 
	0x0002d2, 0x0002e0, 0x0002e5, 0x0002ec, 0x0002ed, 0x0002ee, 0x0002ef, 0x000300, 0x000375, 0x000376, 0x000384, 0x000386, 0x0003f6, 0x0003f7, 0x000482, 0x000483, 
	0x00058d, 0x000590, 0x000606, 0x000609, 0x00060b, 0x00060c, 0x00060e, 0x000610, 0x0006de, 0x0006df, 0x0006e9, 0x0006ea, 0x0006fd, 0x0006ff, 0x0007f6, 0x0007f7, 
	0x0007fe, 0x000800, 0x000888, 0x000889, 0x0009f2, 0x0009f4, 0x0009fa, 0x0009fc, 0x000af1, 0x000af2, 0x000b70, 0x000b71, 0x000bf3, 0x000bfb, 0x000c7f, 0x000c80, 
	0x000d4f, 0x000d50, 0x000d79, 0x000d7a, 0x000e3f, 0x000e40, 0x000f01, 0x000f04, 0x000f13, 0x000f14, 0x000f15, 0x000f18, 0x000f1a, 0x000f20, 0x000f34, 0x000f35, 
	0x000f36, 0x000f37, 0x000f38, 0x000f39, 0x000fbe, 0x000fc6, 0x000fc7, 0x000fcd, 0x000fce, 0x000fd0, 0x000fd5, 0x000fd9, 0x00109e, 0x0010a0, 0x001390, 0x00139a, 
	0x00166d, 0x00166e, 0x0017db, 0x0017dc, 0x001940, 0x001941, 0x0019de, 0x001a00, 0x001b61, 0x001b6b, 0x001b74, 0x001b7d, 0x001fbd, 0x001fbe, 0x001fbf, 0x001fc2, 
	0x001fcd, 0x001fd0, 0x001fdd, 0x001fe0, 0x001fed, 0x001ff0, 0x001ffd, 0x001fff, 0x002044, 0x002045, 0x002052, 0x002053, 0x00207a, 0x00207d, 0x00208a, 0x00208d, 
	0x0020a0, 0x0020c1, 0x002100, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210a, 0x002114, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 0x002126, 
	0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x00212f, 0x00213a, 0x00213c, 0x002140, 0x002145, 0x00214a, 0x00214e, 0x00214f, 0x002150, 0x00218a, 0x00218c, 
	0x002190, 0x002308, 0x00230c, 0x002329, 0x00232b, 0x002427, 0x002440, 0x00244b, 0x00249c, 0x0024ea, 0x002500, 0x002768, 0x002794, 0x0027c5, 0x0027c7, 0x0027e6, 
	0x0027f0, 0x002983, 0x002999, 0x0029d8, 0x0029dc, 0x0029fc, 0x0029fe, 0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002c00, 0x002ce5, 0x002ceb, 0x002e50, 0x002e52, 
	0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x002ff0, 0x002ffc, 0x003004, 0x003005, 0x003012, 0x003014, 0x003020, 0x003021, 0x003036, 0x003038, 
	0x00303e, 0x003040, 0x00309b, 0x00309d, 0x003190, 0x003192, 0x003196, 0x0031a0, 0x0031c0, 0x0031e4, 0x003200, 0x00321f, 0x00322a, 0x003248, 0x003250, 0x003251, 
	0x003260, 0x003280, 0x00328a, 0x0032b1, 0x0032c0, 0x003400, 0x004dc0, 0x004e00, 0x00a490, 0x00a4c7, 0x00a700, 0x00a717, 0x00a720, 0x00a722, 0x00a789, 0x00a78b, 
	0x00a828, 0x00a82c, 0x00a836, 0x00a83a, 0x00aa77, 0x00aa7a, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab6c, 0x00fb29, 0x00fb2a, 0x00fbb2, 0x00fbc3, 0x00fd40, 0x00fd50, 
	0x00fdcf, 0x00fdd0, 0x00fdfc, 0x00fe00, 0x00fe62, 0x00fe63, 0x00fe64, 0x00fe67, 0x00fe69, 0x00fe6a, 0x00ff04, 0x00ff05, 0x00ff0b, 0x00ff0c, 0x00ff1c, 0x00ff1f, 
	0x00ff3e, 0x00ff3f, 0x00ff40, 0x00ff41, 0x00ff5c, 0x00ff5d, 0x00ff5e, 0x00ff5f, 0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fffc, 0x00fffe, 0x010137, 0x010140, 
	0x010179, 0x01018a, 0x01018c, 0x01018f, 0x010190, 0x01019d, 0x0101a0, 0x0101a1, 0x0101d0, 0x0101fd, 0x010877, 0x010879, 0x010ac8, 0x010ac9, 0x01173f, 0x011740, 
	0x011fd5, 0x011ff2, 0x016b3c, 0x016b40, 0x016b45, 0x016b46, 0x01bc9c, 0x01bc9d, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d165, 
	0x01d16a, 0x01d16d, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d1eb, 0x01d200, 0x01d242, 0x01d245, 0x01d246, 0x01d300, 0x01d357, 0x01d6c1, 0x01d6c2, 
	0x01d6db, 0x01d6dc, 0x01d6fb, 0x01d6fc, 0x01d715, 0x01d716, 0x01d735, 0x01d736, 0x01d74f, 0x01d750, 0x01d76f, 0x01d770, 0x01d789, 0x01d78a, 0x01d7a9, 0x01d7aa, 
	0x01d7c3, 0x01d7c4, 0x01d800, 0x01da00, 0x01da37, 0x01da3b, 0x01da6d, 0x01da75, 0x01da76, 0x01da84, 0x01da85, 0x01da87, 0x01e14f, 0x01e150, 0x01e2ff, 0x01e300, 
	0x01ecac, 0x01ecad, 0x01ecb0, 0x01ecb1, 0x01ed2e, 0x01ed2f, 0x01eef0, 0x01eef2, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 
	0x01f0c1, 0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f10d, 0x01f1ae, 0x01f1e6, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01f260, 0x01f266, 
	0x01f300, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 
	0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 
	0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 
};
#define mxCharSet_General_Category_Titlecase_Letter 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Titlecase_Letter[mxCharSet_General_Category_Titlecase_Letter] = {
	0x0001c5, 0x0001c6, 0x0001c8, 0x0001c9, 0x0001cb, 0x0001cc, 0x0001f2, 0x0001f3, 0x001f88, 0x001f90, 0x001f98, 0x001fa0, 0x001fa8, 0x001fb0, 0x001fbc, 0x001fbd, 
	0x001fcc, 0x001fcd, 0x001ffc, 0x001ffd, 
};
#define mxCharSet_General_Category_Unassigned 1414
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Unassigned[mxCharSet_General_Category_Unassigned] = {
	0x000378, 0x00037a, 0x000380, 0x000384, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x000530, 0x000531, 0x000557, 0x000559, 0x00058b, 0x00058d, 
	0x000590, 0x000591, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x000600, 0x00070e, 0x00070f, 0x00074b, 0x00074d, 0x0007b2, 0x0007c0, 0x0007fb, 0x0007fd, 
	0x00082e, 0x000830, 0x00083f, 0x000840, 0x00085c, 0x00085e, 0x00085f, 0x000860, 0x00086b, 0x000870, 0x00088f, 0x000890, 0x000892, 0x000898, 0x000984, 0x000985, 
	0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 0x0009c7, 0x0009c9, 0x0009cb, 
	0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009ff, 0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 
	0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 
	0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a77, 0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 
	0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 
	0x000ae4, 0x000ae6, 0x000af2, 0x000af9, 0x000b00, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 
	0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 
	0x000b78, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 
	0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000be6, 
	0x000bfb, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 
	0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 0x000c66, 0x000c70, 0x000c77, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 
	0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 
	0x000cf0, 0x000cf1, 0x000cf4, 0x000d00, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d50, 0x000d54, 0x000d64, 0x000d66, 
	0x000d80, 0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 0x000dcf, 
	0x000dd5, 0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df5, 0x000e01, 0x000e3b, 0x000e3f, 0x000e5c, 0x000e81, 0x000e83, 0x000e84, 
	0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 0x000ec8, 0x000ecf, 0x000ed0, 
	0x000eda, 0x000edc, 0x000ee0, 0x000f00, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f98, 0x000f99, 0x000fbd, 0x000fbe, 0x000fcd, 0x000fce, 0x000fdb, 0x001000, 
	0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 
	0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 
	0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 0x00135d, 0x00137d, 0x001380, 0x00139a, 0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x001400, 0x00169d, 0x0016a0, 
	0x0016f9, 0x001700, 0x001716, 0x00171f, 0x001737, 0x001740, 0x001754, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 0x001780, 0x0017de, 0x0017e0, 
	0x0017ea, 0x0017f0, 0x0017fa, 0x001800, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x0018b0, 0x0018f6, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 
	0x00193c, 0x001940, 0x001941, 0x001944, 0x00196e, 0x001970, 0x001975, 0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x001a1c, 0x001a1e, 
	0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 0x001ab0, 0x001acf, 0x001b00, 0x001b4d, 0x001b50, 0x001b7f, 0x001b80, 
	0x001bf4, 0x001bfc, 0x001c38, 0x001c3b, 0x001c4a, 0x001c4d, 0x001c89, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc8, 0x001cd0, 0x001cfb, 0x001d00, 0x001f16, 0x001f18, 
	0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 
	0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff6, 0x001fff, 0x002000, 0x002065, 0x002066, 
	0x002072, 0x002074, 0x00208f, 0x002090, 0x00209d, 0x0020a0, 0x0020c1, 0x0020d0, 0x0020f1, 0x002100, 0x00218c, 0x002190, 0x002427, 0x002440, 0x00244b, 0x002460, 
	0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002cf4, 0x002cf9, 0x002d26, 0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 0x002d30, 0x002d68, 0x002d6f, 0x002d71, 0x002d7f, 
	0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 
	0x002ddf, 0x002de0, 0x002e5e, 0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x002ff0, 0x002ffc, 0x003000, 0x003040, 0x003041, 0x003097, 0x003099, 
	0x003100, 0x003105, 0x003130, 0x003131, 0x00318f, 0x003190, 0x0031e4, 0x0031f0, 0x00321f, 0x003220, 0x00a48d, 0x00a490, 0x00a4c7, 0x00a4d0, 0x00a62c, 0x00a640, 
	0x00a6f8, 0x00a700, 0x00a7cb, 0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a82d, 0x00a830, 0x00a83a, 0x00a840, 0x00a878, 0x00a880, 
	0x00a8c6, 0x00a8ce, 0x00a8da, 0x00a8e0, 0x00a954, 0x00a95f, 0x00a97d, 0x00a980, 0x00a9ce, 0x00a9cf, 0x00a9da, 0x00a9de, 0x00a9ff, 0x00aa00, 0x00aa37, 0x00aa40, 
	0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aac3, 0x00aadb, 0x00aaf7, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 
	0x00ab2f, 0x00ab30, 0x00ab6c, 0x00ab70, 0x00abee, 0x00abf0, 0x00abfa, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00d800, 0x00fa6e, 0x00fa70, 
	0x00fada, 0x00fb00, 0x00fb07, 0x00fb13, 0x00fb18, 0x00fb1d, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 0x00fb46, 
	0x00fbc3, 0x00fbd3, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe1a, 0x00fe20, 0x00fe53, 0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 0x00fe70, 
	0x00fe75, 0x00fe76, 0x00fefd, 0x00feff, 0x00ff00, 0x00ff01, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 0x00ffe0, 
	0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fff9, 0x00fffe, 0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 
	0x01005e, 0x010080, 0x0100fb, 0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x01018f, 0x010190, 0x01019d, 0x0101a0, 0x0101a1, 0x0101d0, 0x0101fe, 0x010280, 
	0x01029d, 0x0102a0, 0x0102d1, 0x0102e0, 0x0102fc, 0x010300, 0x010324, 0x01032d, 0x01034b, 0x010350, 0x01037b, 0x010380, 0x01039e, 0x01039f, 0x0103c4, 0x0103c8, 
	0x0103d6, 0x010400, 0x01049e, 0x0104a0, 0x0104aa, 0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 0x010500, 0x010528, 0x010530, 0x010564, 0x01056f, 0x01057b, 0x01057c, 
	0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 0x010600, 0x010737, 0x010740, 
	0x010756, 0x010760, 0x010768, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 
	0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010856, 0x010857, 0x01089f, 0x0108a7, 0x0108b0, 0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x0108fb, 0x01091c, 0x01091f, 
	0x01093a, 0x01093f, 0x010940, 0x010980, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 
	0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a49, 0x010a50, 0x010a59, 0x010a60, 0x010aa0, 0x010ac0, 0x010ae7, 0x010aeb, 0x010af7, 0x010b00, 0x010b36, 0x010b39, 
	0x010b56, 0x010b58, 0x010b73, 0x010b78, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 0x010c00, 0x010c49, 0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010cfa, 
	0x010d28, 0x010d30, 0x010d3a, 0x010e60, 0x010e7f, 0x010e80, 0x010eaa, 0x010eab, 0x010eae, 0x010eb0, 0x010eb2, 0x010efd, 0x010f28, 0x010f30, 0x010f5a, 0x010f70, 
	0x010f8a, 0x010fb0, 0x010fcc, 0x010fe0, 0x010ff7, 0x011000, 0x01104e, 0x011052, 0x011076, 0x01107f, 0x0110c3, 0x0110cd, 0x0110ce, 0x0110d0, 0x0110e9, 0x0110f0, 
	0x0110fa, 0x011100, 0x011135, 0x011136, 0x011148, 0x011150, 0x011177, 0x011180, 0x0111e0, 0x0111e1, 0x0111f5, 0x011200, 0x011212, 0x011213, 0x011242, 0x011280, 
	0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 0x011300, 0x011304, 0x011305, 
	0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133b, 0x011345, 0x011347, 0x011349, 0x01134b, 
	0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011400, 0x01145c, 0x01145d, 0x011462, 0x011480, 
	0x0114c8, 0x0114d0, 0x0114da, 0x011580, 0x0115b6, 0x0115b8, 0x0115de, 0x011600, 0x011645, 0x011650, 0x01165a, 0x011660, 0x01166d, 0x011680, 0x0116ba, 0x0116c0, 
	0x0116ca, 0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x011747, 0x011800, 0x01183c, 0x0118a0, 0x0118f3, 0x0118ff, 0x011907, 0x011909, 0x01190a, 0x01190c, 
	0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011947, 0x011950, 0x01195a, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 
	0x0119e5, 0x011a00, 0x011a48, 0x011a50, 0x011aa3, 0x011ab0, 0x011af9, 0x011b00, 0x011b0a, 0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c46, 0x011c50, 
	0x011c6d, 0x011c70, 0x011c90, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 
	0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 
	0x011daa, 0x011ee0, 0x011ef9, 0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f5a, 0x011fb0, 0x011fb1, 0x011fc0, 0x011ff2, 0x011fff, 0x01239a, 0x012400, 
	0x01246f, 0x012470, 0x012475, 0x012480, 0x012544, 0x012f90, 0x012ff3, 0x013000, 0x013456, 0x014400, 0x014647, 0x016800, 0x016a39, 0x016a40, 0x016a5f, 0x016a60, 
	0x016a6a, 0x016a6e, 0x016abf, 0x016ac0, 0x016aca, 0x016ad0, 0x016aee, 0x016af0, 0x016af6, 0x016b00, 0x016b46, 0x016b50, 0x016b5a, 0x016b5b, 0x016b62, 0x016b63, 
	0x016b78, 0x016b7d, 0x016b90, 0x016e40, 0x016e9b, 0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 0x016fe0, 0x016fe5, 0x016ff0, 0x016ff2, 0x017000, 
	0x0187f8, 0x018800, 0x018cd6, 0x018d00, 0x018d09, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b123, 0x01b132, 0x01b133, 0x01b150, 
	0x01b153, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 0x01b170, 0x01b2fc, 0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9c, 
	0x01bca4, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d1eb, 0x01d200, 0x01d246, 0x01d2c0, 
	0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d300, 0x01d357, 0x01d360, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 
	0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 
	0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 
	0x01da8c, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 
	0x01e025, 0x01e026, 0x01e02b, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 0x01e290, 
	0x01e2af, 0x01e2c0, 0x01e2fa, 0x01e2ff, 0x01e300, 0x01e4d0, 0x01e4fa, 0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 0x01e800, 
	0x01e8c5, 0x01e8c7, 0x01e8d7, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 0x01ec71, 0x01ecb5, 0x01ed01, 0x01ed3e, 0x01ee00, 0x01ee04, 0x01ee05, 
	0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 
	0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 
	0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 
	0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01eef0, 
	0x01eef2, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 0x01f0c1, 0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 
	0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 
	0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 
	0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 
	0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 0x01fbf0, 0x01fbfa, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 
	0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 0x0e0100, 
	0x0e01f0, 0x0f0000, 0x0ffffe, 0x100000, 0x10fffe, 0x110000, 
};
#define mxCharSet_General_Category_Uppercase_Letter 1292
static const txInteger ICACHE_RODATA_ATTR gxCharSet_General_Category_Uppercase_Letter[mxCharSet_General_Category_Uppercase_Letter] = {
	0x000041, 0x00005b, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000df, 0x000100, 0x000101, 0x000102, 0x000103, 0x000104, 0x000105, 0x000106, 0x000107, 0x000108, 0x000109, 
	0x00010a, 0x00010b, 0x00010c, 0x00010d, 0x00010e, 0x00010f, 0x000110, 0x000111, 0x000112, 0x000113, 0x000114, 0x000115, 0x000116, 0x000117, 0x000118, 0x000119, 
	0x00011a, 0x00011b, 0x00011c, 0x00011d, 0x00011e, 0x00011f, 0x000120, 0x000121, 0x000122, 0x000123, 0x000124, 0x000125, 0x000126, 0x000127, 0x000128, 0x000129, 
	0x00012a, 0x00012b, 0x00012c, 0x00012d, 0x00012e, 0x00012f, 0x000130, 0x000131, 0x000132, 0x000133, 0x000134, 0x000135, 0x000136, 0x000137, 0x000139, 0x00013a, 
	0x00013b, 0x00013c, 0x00013d, 0x00013e, 0x00013f, 0x000140, 0x000141, 0x000142, 0x000143, 0x000144, 0x000145, 0x000146, 0x000147, 0x000148, 0x00014a, 0x00014b, 
	0x00014c, 0x00014d, 0x00014e, 0x00014f, 0x000150, 0x000151, 0x000152, 0x000153, 0x000154, 0x000155, 0x000156, 0x000157, 0x000158, 0x000159, 0x00015a, 0x00015b, 
	0x00015c, 0x00015d, 0x00015e, 0x00015f, 0x000160, 0x000161, 0x000162, 0x000163, 0x000164, 0x000165, 0x000166, 0x000167, 0x000168, 0x000169, 0x00016a, 0x00016b, 
	0x00016c, 0x00016d, 0x00016e, 0x00016f, 0x000170, 0x000171, 0x000172, 0x000173, 0x000174, 0x000175, 0x000176, 0x000177, 0x000178, 0x00017a, 0x00017b, 0x00017c, 
	0x00017d, 0x00017e, 0x000181, 0x000183, 0x000184, 0x000185, 0x000186, 0x000188, 0x000189, 0x00018c, 0x00018e, 0x000192, 0x000193, 0x000195, 0x000196, 0x000199, 
	0x00019c, 0x00019e, 0x00019f, 0x0001a1, 0x0001a2, 0x0001a3, 0x0001a4, 0x0001a5, 0x0001a6, 0x0001a8, 0x0001a9, 0x0001aa, 0x0001ac, 0x0001ad, 0x0001ae, 0x0001b0, 
	0x0001b1, 0x0001b4, 0x0001b5, 0x0001b6, 0x0001b7, 0x0001b9, 0x0001bc, 0x0001bd, 0x0001c4, 0x0001c5, 0x0001c7, 0x0001c8, 0x0001ca, 0x0001cb, 0x0001cd, 0x0001ce, 
	0x0001cf, 0x0001d0, 0x0001d1, 0x0001d2, 0x0001d3, 0x0001d4, 0x0001d5, 0x0001d6, 0x0001d7, 0x0001d8, 0x0001d9, 0x0001da, 0x0001db, 0x0001dc, 0x0001de, 0x0001df, 
	0x0001e0, 0x0001e1, 0x0001e2, 0x0001e3, 0x0001e4, 0x0001e5, 0x0001e6, 0x0001e7, 0x0001e8, 0x0001e9, 0x0001ea, 0x0001eb, 0x0001ec, 0x0001ed, 0x0001ee, 0x0001ef, 
	0x0001f1, 0x0001f2, 0x0001f4, 0x0001f5, 0x0001f6, 0x0001f9, 0x0001fa, 0x0001fb, 0x0001fc, 0x0001fd, 0x0001fe, 0x0001ff, 0x000200, 0x000201, 0x000202, 0x000203, 
	0x000204, 0x000205, 0x000206, 0x000207, 0x000208, 0x000209, 0x00020a, 0x00020b, 0x00020c, 0x00020d, 0x00020e, 0x00020f, 0x000210, 0x000211, 0x000212, 0x000213, 
	0x000214, 0x000215, 0x000216, 0x000217, 0x000218, 0x000219, 0x00021a, 0x00021b, 0x00021c, 0x00021d, 0x00021e, 0x00021f, 0x000220, 0x000221, 0x000222, 0x000223, 
	0x000224, 0x000225, 0x000226, 0x000227, 0x000228, 0x000229, 0x00022a, 0x00022b, 0x00022c, 0x00022d, 0x00022e, 0x00022f, 0x000230, 0x000231, 0x000232, 0x000233, 
	0x00023a, 0x00023c, 0x00023d, 0x00023f, 0x000241, 0x000242, 0x000243, 0x000247, 0x000248, 0x000249, 0x00024a, 0x00024b, 0x00024c, 0x00024d, 0x00024e, 0x00024f, 
	0x000370, 0x000371, 0x000372, 0x000373, 0x000376, 0x000377, 0x00037f, 0x000380, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x000390, 
	0x000391, 0x0003a2, 0x0003a3, 0x0003ac, 0x0003cf, 0x0003d0, 0x0003d2, 0x0003d5, 0x0003d8, 0x0003d9, 0x0003da, 0x0003db, 0x0003dc, 0x0003dd, 0x0003de, 0x0003df, 
	0x0003e0, 0x0003e1, 0x0003e2, 0x0003e3, 0x0003e4, 0x0003e5, 0x0003e6, 0x0003e7, 0x0003e8, 0x0003e9, 0x0003ea, 0x0003eb, 0x0003ec, 0x0003ed, 0x0003ee, 0x0003ef, 
	0x0003f4, 0x0003f5, 0x0003f7, 0x0003f8, 0x0003f9, 0x0003fb, 0x0003fd, 0x000430, 0x000460, 0x000461, 0x000462, 0x000463, 0x000464, 0x000465, 0x000466, 0x000467, 
	0x000468, 0x000469, 0x00046a, 0x00046b, 0x00046c, 0x00046d, 0x00046e, 0x00046f, 0x000470, 0x000471, 0x000472, 0x000473, 0x000474, 0x000475, 0x000476, 0x000477, 
	0x000478, 0x000479, 0x00047a, 0x00047b, 0x00047c, 0x00047d, 0x00047e, 0x00047f, 0x000480, 0x000481, 0x00048a, 0x00048b, 0x00048c, 0x00048d, 0x00048e, 0x00048f, 
	0x000490, 0x000491, 0x000492, 0x000493, 0x000494, 0x000495, 0x000496, 0x000497, 0x000498, 0x000499, 0x00049a, 0x00049b, 0x00049c, 0x00049d, 0x00049e, 0x00049f, 
	0x0004a0, 0x0004a1, 0x0004a2, 0x0004a3, 0x0004a4, 0x0004a5, 0x0004a6, 0x0004a7, 0x0004a8, 0x0004a9, 0x0004aa, 0x0004ab, 0x0004ac, 0x0004ad, 0x0004ae, 0x0004af, 
	0x0004b0, 0x0004b1, 0x0004b2, 0x0004b3, 0x0004b4, 0x0004b5, 0x0004b6, 0x0004b7, 0x0004b8, 0x0004b9, 0x0004ba, 0x0004bb, 0x0004bc, 0x0004bd, 0x0004be, 0x0004bf, 
	0x0004c0, 0x0004c2, 0x0004c3, 0x0004c4, 0x0004c5, 0x0004c6, 0x0004c7, 0x0004c8, 0x0004c9, 0x0004ca, 0x0004cb, 0x0004cc, 0x0004cd, 0x0004ce, 0x0004d0, 0x0004d1, 
	0x0004d2, 0x0004d3, 0x0004d4, 0x0004d5, 0x0004d6, 0x0004d7, 0x0004d8, 0x0004d9, 0x0004da, 0x0004db, 0x0004dc, 0x0004dd, 0x0004de, 0x0004df, 0x0004e0, 0x0004e1, 
	0x0004e2, 0x0004e3, 0x0004e4, 0x0004e5, 0x0004e6, 0x0004e7, 0x0004e8, 0x0004e9, 0x0004ea, 0x0004eb, 0x0004ec, 0x0004ed, 0x0004ee, 0x0004ef, 0x0004f0, 0x0004f1, 
	0x0004f2, 0x0004f3, 0x0004f4, 0x0004f5, 0x0004f6, 0x0004f7, 0x0004f8, 0x0004f9, 0x0004fa, 0x0004fb, 0x0004fc, 0x0004fd, 0x0004fe, 0x0004ff, 0x000500, 0x000501, 
	0x000502, 0x000503, 0x000504, 0x000505, 0x000506, 0x000507, 0x000508, 0x000509, 0x00050a, 0x00050b, 0x00050c, 0x00050d, 0x00050e, 0x00050f, 0x000510, 0x000511, 
	0x000512, 0x000513, 0x000514, 0x000515, 0x000516, 0x000517, 0x000518, 0x000519, 0x00051a, 0x00051b, 0x00051c, 0x00051d, 0x00051e, 0x00051f, 0x000520, 0x000521, 
	0x000522, 0x000523, 0x000524, 0x000525, 0x000526, 0x000527, 0x000528, 0x000529, 0x00052a, 0x00052b, 0x00052c, 0x00052d, 0x00052e, 0x00052f, 0x000531, 0x000557, 
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0013a0, 0x0013f6, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x001e00, 0x001e01, 0x001e02, 0x001e03, 
	0x001e04, 0x001e05, 0x001e06, 0x001e07, 0x001e08, 0x001e09, 0x001e0a, 0x001e0b, 0x001e0c, 0x001e0d, 0x001e0e, 0x001e0f, 0x001e10, 0x001e11, 0x001e12, 0x001e13, 
	0x001e14, 0x001e15, 0x001e16, 0x001e17, 0x001e18, 0x001e19, 0x001e1a, 0x001e1b, 0x001e1c, 0x001e1d, 0x001e1e, 0x001e1f, 0x001e20, 0x001e21, 0x001e22, 0x001e23, 
	0x001e24, 0x001e25, 0x001e26, 0x001e27, 0x001e28, 0x001e29, 0x001e2a, 0x001e2b, 0x001e2c, 0x001e2d, 0x001e2e, 0x001e2f, 0x001e30, 0x001e31, 0x001e32, 0x001e33, 
	0x001e34, 0x001e35, 0x001e36, 0x001e37, 0x001e38, 0x001e39, 0x001e3a, 0x001e3b, 0x001e3c, 0x001e3d, 0x001e3e, 0x001e3f, 0x001e40, 0x001e41, 0x001e42, 0x001e43, 
	0x001e44, 0x001e45, 0x001e46, 0x001e47, 0x001e48, 0x001e49, 0x001e4a, 0x001e4b, 0x001e4c, 0x001e4d, 0x001e4e, 0x001e4f, 0x001e50, 0x001e51, 0x001e52, 0x001e53, 
	0x001e54, 0x001e55, 0x001e56, 0x001e57, 0x001e58, 0x001e59, 0x001e5a, 0x001e5b, 0x001e5c, 0x001e5d, 0x001e5e, 0x001e5f, 0x001e60, 0x001e61, 0x001e62, 0x001e63, 
	0x001e64, 0x001e65, 0x001e66, 0x001e67, 0x001e68, 0x001e69, 0x001e6a, 0x001e6b, 0x001e6c, 0x001e6d, 0x001e6e, 0x001e6f, 0x001e70, 0x001e71, 0x001e72, 0x001e73, 
	0x001e74, 0x001e75, 0x001e76, 0x001e77, 0x001e78, 0x001e79, 0x001e7a, 0x001e7b, 0x001e7c, 0x001e7d, 0x001e7e, 0x001e7f, 0x001e80, 0x001e81, 0x001e82, 0x001e83, 
	0x001e84, 0x001e85, 0x001e86, 0x001e87, 0x001e88, 0x001e89, 0x001e8a, 0x001e8b, 0x001e8c, 0x001e8d, 0x001e8e, 0x001e8f, 0x001e90, 0x001e91, 0x001e92, 0x001e93, 
	0x001e94, 0x001e95, 0x001e9e, 0x001e9f, 0x001ea0, 0x001ea1, 0x001ea2, 0x001ea3, 0x001ea4, 0x001ea5, 0x001ea6, 0x001ea7, 0x001ea8, 0x001ea9, 0x001eaa, 0x001eab, 
	0x001eac, 0x001ead, 0x001eae, 0x001eaf, 0x001eb0, 0x001eb1, 0x001eb2, 0x001eb3, 0x001eb4, 0x001eb5, 0x001eb6, 0x001eb7, 0x001eb8, 0x001eb9, 0x001eba, 0x001ebb, 
	0x001ebc, 0x001ebd, 0x001ebe, 0x001ebf, 0x001ec0, 0x001ec1, 0x001ec2, 0x001ec3, 0x001ec4, 0x001ec5, 0x001ec6, 0x001ec7, 0x001ec8, 0x001ec9, 0x001eca, 0x001ecb, 
	0x001ecc, 0x001ecd, 0x001ece, 0x001ecf, 0x001ed0, 0x001ed1, 0x001ed2, 0x001ed3, 0x001ed4, 0x001ed5, 0x001ed6, 0x001ed7, 0x001ed8, 0x001ed9, 0x001eda, 0x001edb, 
	0x001edc, 0x001edd, 0x001ede, 0x001edf, 0x001ee0, 0x001ee1, 0x001ee2, 0x001ee3, 0x001ee4, 0x001ee5, 0x001ee6, 0x001ee7, 0x001ee8, 0x001ee9, 0x001eea, 0x001eeb, 
	0x001eec, 0x001eed, 0x001eee, 0x001eef, 0x001ef0, 0x001ef1, 0x001ef2, 0x001ef3, 0x001ef4, 0x001ef5, 0x001ef6, 0x001ef7, 0x001ef8, 0x001ef9, 0x001efa, 0x001efb, 
	0x001efc, 0x001efd, 0x001efe, 0x001eff, 0x001f08, 0x001f10, 0x001f18, 0x001f1e, 0x001f28, 0x001f30, 0x001f38, 0x001f40, 0x001f48, 0x001f4e, 0x001f59, 0x001f5a, 
	0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f60, 0x001f68, 0x001f70, 0x001fb8, 0x001fbc, 0x001fc8, 0x001fcc, 0x001fd8, 0x001fdc, 0x001fe8, 0x001fed, 
	0x001ff8, 0x001ffc, 0x002102, 0x002103, 0x002107, 0x002108, 0x00210b, 0x00210e, 0x002110, 0x002113, 0x002115, 0x002116, 0x002119, 0x00211e, 0x002124, 0x002125, 
	0x002126, 0x002127, 0x002128, 0x002129, 0x00212a, 0x00212e, 0x002130, 0x002134, 0x00213e, 0x002140, 0x002145, 0x002146, 0x002183, 0x002184, 0x002c00, 0x002c30, 
	0x002c60, 0x002c61, 0x002c62, 0x002c65, 0x002c67, 0x002c68, 0x002c69, 0x002c6a, 0x002c6b, 0x002c6c, 0x002c6d, 0x002c71, 0x002c72, 0x002c73, 0x002c75, 0x002c76, 
	0x002c7e, 0x002c81, 0x002c82, 0x002c83, 0x002c84, 0x002c85, 0x002c86, 0x002c87, 0x002c88, 0x002c89, 0x002c8a, 0x002c8b, 0x002c8c, 0x002c8d, 0x002c8e, 0x002c8f, 
	0x002c90, 0x002c91, 0x002c92, 0x002c93, 0x002c94, 0x002c95, 0x002c96, 0x002c97, 0x002c98, 0x002c99, 0x002c9a, 0x002c9b, 0x002c9c, 0x002c9d, 0x002c9e, 0x002c9f, 
	0x002ca0, 0x002ca1, 0x002ca2, 0x002ca3, 0x002ca4, 0x002ca5, 0x002ca6, 0x002ca7, 0x002ca8, 0x002ca9, 0x002caa, 0x002cab, 0x002cac, 0x002cad, 0x002cae, 0x002caf, 
	0x002cb0, 0x002cb1, 0x002cb2, 0x002cb3, 0x002cb4, 0x002cb5, 0x002cb6, 0x002cb7, 0x002cb8, 0x002cb9, 0x002cba, 0x002cbb, 0x002cbc, 0x002cbd, 0x002cbe, 0x002cbf, 
	0x002cc0, 0x002cc1, 0x002cc2, 0x002cc3, 0x002cc4, 0x002cc5, 0x002cc6, 0x002cc7, 0x002cc8, 0x002cc9, 0x002cca, 0x002ccb, 0x002ccc, 0x002ccd, 0x002cce, 0x002ccf, 
	0x002cd0, 0x002cd1, 0x002cd2, 0x002cd3, 0x002cd4, 0x002cd5, 0x002cd6, 0x002cd7, 0x002cd8, 0x002cd9, 0x002cda, 0x002cdb, 0x002cdc, 0x002cdd, 0x002cde, 0x002cdf, 
	0x002ce0, 0x002ce1, 0x002ce2, 0x002ce3, 0x002ceb, 0x002cec, 0x002ced, 0x002cee, 0x002cf2, 0x002cf3, 0x00a640, 0x00a641, 0x00a642, 0x00a643, 0x00a644, 0x00a645, 
	0x00a646, 0x00a647, 0x00a648, 0x00a649, 0x00a64a, 0x00a64b, 0x00a64c, 0x00a64d, 0x00a64e, 0x00a64f, 0x00a650, 0x00a651, 0x00a652, 0x00a653, 0x00a654, 0x00a655, 
	0x00a656, 0x00a657, 0x00a658, 0x00a659, 0x00a65a, 0x00a65b, 0x00a65c, 0x00a65d, 0x00a65e, 0x00a65f, 0x00a660, 0x00a661, 0x00a662, 0x00a663, 0x00a664, 0x00a665, 
	0x00a666, 0x00a667, 0x00a668, 0x00a669, 0x00a66a, 0x00a66b, 0x00a66c, 0x00a66d, 0x00a680, 0x00a681, 0x00a682, 0x00a683, 0x00a684, 0x00a685, 0x00a686, 0x00a687, 
	0x00a688, 0x00a689, 0x00a68a, 0x00a68b, 0x00a68c, 0x00a68d, 0x00a68e, 0x00a68f, 0x00a690, 0x00a691, 0x00a692, 0x00a693, 0x00a694, 0x00a695, 0x00a696, 0x00a697, 
	0x00a698, 0x00a699, 0x00a69a, 0x00a69b, 0x00a722, 0x00a723, 0x00a724, 0x00a725, 0x00a726, 0x00a727, 0x00a728, 0x00a729, 0x00a72a, 0x00a72b, 0x00a72c, 0x00a72d, 
	0x00a72e, 0x00a72f, 0x00a732, 0x00a733, 0x00a734, 0x00a735, 0x00a736, 0x00a737, 0x00a738, 0x00a739, 0x00a73a, 0x00a73b, 0x00a73c, 0x00a73d, 0x00a73e, 0x00a73f, 
	0x00a740, 0x00a741, 0x00a742, 0x00a743, 0x00a744, 0x00a745, 0x00a746, 0x00a747, 0x00a748, 0x00a749, 0x00a74a, 0x00a74b, 0x00a74c, 0x00a74d, 0x00a74e, 0x00a74f, 
	0x00a750, 0x00a751, 0x00a752, 0x00a753, 0x00a754, 0x00a755, 0x00a756, 0x00a757, 0x00a758, 0x00a759, 0x00a75a, 0x00a75b, 0x00a75c, 0x00a75d, 0x00a75e, 0x00a75f, 
	0x00a760, 0x00a761, 0x00a762, 0x00a763, 0x00a764, 0x00a765, 0x00a766, 0x00a767, 0x00a768, 0x00a769, 0x00a76a, 0x00a76b, 0x00a76c, 0x00a76d, 0x00a76e, 0x00a76f, 
	0x00a779, 0x00a77a, 0x00a77b, 0x00a77c, 0x00a77d, 0x00a77f, 0x00a780, 0x00a781, 0x00a782, 0x00a783, 0x00a784, 0x00a785, 0x00a786, 0x00a787, 0x00a78b, 0x00a78c, 
	0x00a78d, 0x00a78e, 0x00a790, 0x00a791, 0x00a792, 0x00a793, 0x00a796, 0x00a797, 0x00a798, 0x00a799, 0x00a79a, 0x00a79b, 0x00a79c, 0x00a79d, 0x00a79e, 0x00a79f, 
	0x00a7a0, 0x00a7a1, 0x00a7a2, 0x00a7a3, 0x00a7a4, 0x00a7a5, 0x00a7a6, 0x00a7a7, 0x00a7a8, 0x00a7a9, 0x00a7aa, 0x00a7af, 0x00a7b0, 0x00a7b5, 0x00a7b6, 0x00a7b7, 
	0x00a7b8, 0x00a7b9, 0x00a7ba, 0x00a7bb, 0x00a7bc, 0x00a7bd, 0x00a7be, 0x00a7bf, 0x00a7c0, 0x00a7c1, 0x00a7c2, 0x00a7c3, 0x00a7c4, 0x00a7c8, 0x00a7c9, 0x00a7ca, 
	0x00a7d0, 0x00a7d1, 0x00a7d6, 0x00a7d7, 0x00a7d8, 0x00a7d9, 0x00a7f5, 0x00a7f6, 0x00ff21, 0x00ff3b, 0x010400, 0x010428, 0x0104b0, 0x0104d4, 0x010570, 0x01057b, 
	0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010c80, 0x010cb3, 0x0118a0, 0x0118c0, 0x016e40, 0x016e60, 0x01d400, 0x01d41a, 0x01d434, 0x01d44e, 
	0x01d468, 0x01d482, 0x01d49c, 0x01d49d, 0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4b6, 0x01d4d0, 0x01d4ea, 
	0x01d504, 0x01d506, 0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d538, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 
	0x01d54a, 0x01d551, 0x01d56c, 0x01d586, 0x01d5a0, 0x01d5ba, 0x01d5d4, 0x01d5ee, 0x01d608, 0x01d622, 0x01d63c, 0x01d656, 0x01d670, 0x01d68a, 0x01d6a8, 0x01d6c1, 
	0x01d6e2, 0x01d6fb, 0x01d71c, 0x01d735, 0x01d756, 0x01d76f, 0x01d790, 0x01d7a9, 0x01d7ca, 0x01d7cb, 0x01e900, 0x01e922, 
};
#define mxCharSet_Script_Adlam 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Adlam[mxCharSet_Script_Adlam] = {
	0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 
};
#define mxCharSet_Script_Ahom 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Ahom[mxCharSet_Script_Ahom] = {
	0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x011747, 
};
#define mxCharSet_Script_Anatolian_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Anatolian_Hieroglyphs[mxCharSet_Script_Anatolian_Hieroglyphs] = {
	0x014400, 0x014647, 
};
#define mxCharSet_Script_Arabic 116
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Arabic[mxCharSet_Script_Arabic] = {
	0x000600, 0x000605, 0x000606, 0x00060c, 0x00060d, 0x00061b, 0x00061c, 0x00061f, 0x000620, 0x000640, 0x000641, 0x00064b, 0x000656, 0x000670, 0x000671, 0x0006dd, 
	0x0006de, 0x000700, 0x000750, 0x000780, 0x000870, 0x00088f, 0x000890, 0x000892, 0x000898, 0x0008e2, 0x0008e3, 0x000900, 0x00fb50, 0x00fbc3, 0x00fbd3, 0x00fd3e, 
	0x00fd40, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe00, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x010e60, 0x010e7f, 0x010efd, 0x010f00, 
	0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 
	0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 
	0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 
	0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 
	0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 
};
#define mxCharSet_Script_Armenian 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Armenian[mxCharSet_Script_Armenian] = {
	0x000531, 0x000557, 0x000559, 0x00058b, 0x00058d, 0x000590, 0x00fb13, 0x00fb18, 
};
#define mxCharSet_Script_Avestan 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Avestan[mxCharSet_Script_Avestan] = {
	0x010b00, 0x010b36, 0x010b39, 0x010b40, 
};
#define mxCharSet_Script_Balinese 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Balinese[mxCharSet_Script_Balinese] = {
	0x001b00, 0x001b4d, 0x001b50, 0x001b7f, 
};
#define mxCharSet_Script_Bamum 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Bamum[mxCharSet_Script_Bamum] = {
	0x00a6a0, 0x00a6f8, 0x016800, 0x016a39, 
};
#define mxCharSet_Script_Bassa_Vah 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Bassa_Vah[mxCharSet_Script_Bassa_Vah] = {
	0x016ad0, 0x016aee, 0x016af0, 0x016af6, 
};
#define mxCharSet_Script_Batak 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Batak[mxCharSet_Script_Batak] = {
	0x001bc0, 0x001bf4, 0x001bfc, 0x001c00, 
};
#define mxCharSet_Script_Bengali 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Bengali[mxCharSet_Script_Bengali] = {
	0x000980, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 
	0x0009c7, 0x0009c9, 0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009ff, 
};
#define mxCharSet_Script_Bhaiksuki 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Bhaiksuki[mxCharSet_Script_Bhaiksuki] = {
	0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c46, 0x011c50, 0x011c6d, 
};
#define mxCharSet_Script_Bopomofo 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Bopomofo[mxCharSet_Script_Bopomofo] = {
	0x0002ea, 0x0002ec, 0x003105, 0x003130, 0x0031a0, 0x0031c0, 
};
#define mxCharSet_Script_Brahmi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Brahmi[mxCharSet_Script_Brahmi] = {
	0x011000, 0x01104e, 0x011052, 0x011076, 0x01107f, 0x011080, 
};
#define mxCharSet_Script_Braille 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Braille[mxCharSet_Script_Braille] = {
	0x002800, 0x002900, 
};
#define mxCharSet_Script_Buginese 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Buginese[mxCharSet_Script_Buginese] = {
	0x001a00, 0x001a1c, 0x001a1e, 0x001a20, 
};
#define mxCharSet_Script_Buhid 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Buhid[mxCharSet_Script_Buhid] = {
	0x001740, 0x001754, 
};
#define mxCharSet_Script_Canadian_Aboriginal 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Canadian_Aboriginal[mxCharSet_Script_Canadian_Aboriginal] = {
	0x001400, 0x001680, 0x0018b0, 0x0018f6, 0x011ab0, 0x011ac0, 
};
#define mxCharSet_Script_Carian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Carian[mxCharSet_Script_Carian] = {
	0x0102a0, 0x0102d1, 
};
#define mxCharSet_Script_Caucasian_Albanian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Caucasian_Albanian[mxCharSet_Script_Caucasian_Albanian] = {
	0x010530, 0x010564, 0x01056f, 0x010570, 
};
#define mxCharSet_Script_Chakma 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Chakma[mxCharSet_Script_Chakma] = {
	0x011100, 0x011135, 0x011136, 0x011148, 
};
#define mxCharSet_Script_Cham 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cham[mxCharSet_Script_Cham] = {
	0x00aa00, 0x00aa37, 0x00aa40, 0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aa60, 
};
#define mxCharSet_Script_Cherokee 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cherokee[mxCharSet_Script_Cherokee] = {
	0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x00ab70, 0x00abc0, 
};
#define mxCharSet_Script_Chorasmian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Chorasmian[mxCharSet_Script_Chorasmian] = {
	0x010fb0, 0x010fcc, 
};
#define mxCharSet_Script_Common 346
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Common[mxCharSet_Script_Common] = {
	0x000000, 0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002b9, 0x0002e0, 
	0x0002e5, 0x0002ea, 0x0002ec, 0x000300, 0x000374, 0x000375, 0x00037e, 0x00037f, 0x000385, 0x000386, 0x000387, 0x000388, 0x000605, 0x000606, 0x00060c, 0x00060d, 
	0x00061b, 0x00061c, 0x00061f, 0x000620, 0x000640, 0x000641, 0x0006dd, 0x0006de, 0x0008e2, 0x0008e3, 0x000964, 0x000966, 0x000e3f, 0x000e40, 0x000fd5, 0x000fd9, 
	0x0010fb, 0x0010fc, 0x0016eb, 0x0016ee, 0x001735, 0x001737, 0x001802, 0x001804, 0x001805, 0x001806, 0x001cd3, 0x001cd4, 0x001ce1, 0x001ce2, 0x001ce9, 0x001ced, 
	0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001cfb, 0x002000, 0x00200c, 0x00200e, 0x002065, 0x002066, 0x002071, 0x002074, 0x00207f, 0x002080, 0x00208f, 
	0x0020a0, 0x0020c1, 0x002100, 0x002126, 0x002127, 0x00212a, 0x00212c, 0x002132, 0x002133, 0x00214e, 0x00214f, 0x002160, 0x002189, 0x00218c, 0x002190, 0x002427, 
	0x002440, 0x00244b, 0x002460, 0x002800, 0x002900, 0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002c00, 0x002e00, 0x002e5e, 0x002ff0, 0x002ffc, 0x003000, 0x003005, 
	0x003006, 0x003007, 0x003008, 0x003021, 0x003030, 0x003038, 0x00303c, 0x003040, 0x00309b, 0x00309d, 0x0030a0, 0x0030a1, 0x0030fb, 0x0030fd, 0x003190, 0x0031a0, 
	0x0031c0, 0x0031e4, 0x003220, 0x003260, 0x00327f, 0x0032d0, 0x0032ff, 0x003300, 0x003358, 0x003400, 0x004dc0, 0x004e00, 0x00a700, 0x00a722, 0x00a788, 0x00a78b, 
	0x00a830, 0x00a83a, 0x00a92e, 0x00a92f, 0x00a9cf, 0x00a9d0, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab6c, 0x00fd3e, 0x00fd40, 0x00fe10, 0x00fe1a, 0x00fe30, 0x00fe53, 
	0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 0x00feff, 0x00ff00, 0x00ff01, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff66, 0x00ff70, 0x00ff71, 0x00ff9e, 0x00ffa0, 
	0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fff9, 0x00fffe, 0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x010140, 0x010190, 0x01019d, 0x0101d0, 0x0101fd, 
	0x0102e1, 0x0102fc, 0x01bca0, 0x01bca4, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d167, 0x01d16a, 0x01d17b, 0x01d183, 0x01d185, 
	0x01d18c, 0x01d1aa, 0x01d1ae, 0x01d1eb, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d300, 0x01d357, 0x01d360, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 
	0x01d49e, 0x01d4a0, 0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 
	0x01d507, 0x01d50b, 0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 
	0x01d552, 0x01d6a6, 0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01d800, 0x01ec71, 0x01ecb5, 0x01ed01, 0x01ed3e, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 
	0x01f0b1, 0x01f0c0, 0x01f0c1, 0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 0x01f200, 0x01f201, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 
	0x01f250, 0x01f252, 0x01f260, 0x01f266, 0x01f300, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 
	0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 
	0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 
	0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 0x01fbf0, 0x01fbfa, 0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 
};
#define mxCharSet_Script_Coptic 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Coptic[mxCharSet_Script_Coptic] = {
	0x0003e2, 0x0003f0, 0x002c80, 0x002cf4, 0x002cf9, 0x002d00, 
};
#define mxCharSet_Script_Cuneiform 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cuneiform[mxCharSet_Script_Cuneiform] = {
	0x012000, 0x01239a, 0x012400, 0x01246f, 0x012470, 0x012475, 0x012480, 0x012544, 
};
#define mxCharSet_Script_Cypriot 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cypriot[mxCharSet_Script_Cypriot] = {
	0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 0x01083f, 0x010840, 
};
#define mxCharSet_Script_Cypro_Minoan 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cypro_Minoan[mxCharSet_Script_Cypro_Minoan] = {
	0x012f90, 0x012ff3, 
};
#define mxCharSet_Script_Cyrillic 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Cyrillic[mxCharSet_Script_Cyrillic] = {
	0x000400, 0x000485, 0x000487, 0x000530, 0x001c80, 0x001c89, 0x001d2b, 0x001d2c, 0x001d78, 0x001d79, 0x002de0, 0x002e00, 0x00a640, 0x00a6a0, 0x00fe2e, 0x00fe30, 
	0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 
};
#define mxCharSet_Script_Deseret 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Deseret[mxCharSet_Script_Deseret] = {
	0x010400, 0x010450, 
};
#define mxCharSet_Script_Devanagari 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Devanagari[mxCharSet_Script_Devanagari] = {
	0x000900, 0x000951, 0x000955, 0x000964, 0x000966, 0x000980, 0x00a8e0, 0x00a900, 0x011b00, 0x011b0a, 
};
#define mxCharSet_Script_Dives_Akuru 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Dives_Akuru[mxCharSet_Script_Dives_Akuru] = {
	0x011900, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011947, 0x011950, 0x01195a, 
};
#define mxCharSet_Script_Dogra 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Dogra[mxCharSet_Script_Dogra] = {
	0x011800, 0x01183c, 
};
#define mxCharSet_Script_Duployan 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Duployan[mxCharSet_Script_Duployan] = {
	0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9c, 0x01bca0, 
};
#define mxCharSet_Script_Egyptian_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Egyptian_Hieroglyphs[mxCharSet_Script_Egyptian_Hieroglyphs] = {
	0x013000, 0x013456, 
};
#define mxCharSet_Script_Elbasan 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Elbasan[mxCharSet_Script_Elbasan] = {
	0x010500, 0x010528, 
};
#define mxCharSet_Script_Elymaic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Elymaic[mxCharSet_Script_Elymaic] = {
	0x010fe0, 0x010ff7, 
};
#define mxCharSet_Script_Ethiopic 72
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Ethiopic[mxCharSet_Script_Ethiopic] = {
	0x001200, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 
	0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 
	0x00135d, 0x00137d, 0x001380, 0x00139a, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 
	0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 
	0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 
};
#define mxCharSet_Script_Georgian 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Georgian[mxCharSet_Script_Georgian] = {
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x0010fb, 0x0010fc, 0x001100, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x002d00, 0x002d26, 
	0x002d27, 0x002d28, 0x002d2d, 0x002d2e, 
};
#define mxCharSet_Script_Glagolitic 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Glagolitic[mxCharSet_Script_Glagolitic] = {
	0x002c00, 0x002c60, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 0x01e023, 0x01e025, 0x01e026, 0x01e02b, 
};
#define mxCharSet_Script_Gothic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Gothic[mxCharSet_Script_Gothic] = {
	0x010330, 0x01034b, 
};
#define mxCharSet_Script_Grantha 30
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Grantha[mxCharSet_Script_Grantha] = {
	0x011300, 0x011304, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133c, 0x011345, 
	0x011347, 0x011349, 0x01134b, 0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 
};
#define mxCharSet_Script_Greek 72
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Greek[mxCharSet_Script_Greek] = {
	0x000370, 0x000374, 0x000375, 0x000378, 0x00037a, 0x00037e, 0x00037f, 0x000380, 0x000384, 0x000385, 0x000386, 0x000387, 0x000388, 0x00038b, 0x00038c, 0x00038d, 
	0x00038e, 0x0003a2, 0x0003a3, 0x0003e2, 0x0003f0, 0x000400, 0x001d26, 0x001d2b, 0x001d5d, 0x001d62, 0x001d66, 0x001d6b, 0x001dbf, 0x001dc0, 0x001f00, 0x001f16, 
	0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 
	0x001f80, 0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 0x001ff6, 0x001fff, 0x002126, 0x002127, 
	0x00ab65, 0x00ab66, 0x010140, 0x01018f, 0x0101a0, 0x0101a1, 0x01d200, 0x01d246, 
};
#define mxCharSet_Script_Gujarati 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Gujarati[mxCharSet_Script_Gujarati] = {
	0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 
	0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 0x000ae6, 0x000af2, 0x000af9, 0x000b00, 
};
#define mxCharSet_Script_Gunjala_Gondi 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Gunjala_Gondi[mxCharSet_Script_Gunjala_Gondi] = {
	0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 0x011daa, 
};
#define mxCharSet_Script_Gurmukhi 32
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Gurmukhi[mxCharSet_Script_Gurmukhi] = {
	0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 0x000a35, 0x000a37, 0x000a38, 0x000a3a, 
	0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 0x000a5e, 0x000a5f, 0x000a66, 0x000a77, 
};
#define mxCharSet_Script_Han 42
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Han[mxCharSet_Script_Han] = {
	0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x003005, 0x003006, 0x003007, 0x003008, 0x003021, 0x00302a, 0x003038, 0x00303c, 0x003400, 0x004dc0, 
	0x004e00, 0x00a000, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 0x016fe2, 0x016fe4, 0x016ff0, 0x016ff2, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 0x02b740, 0x02b81e, 
	0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Script_Hangul 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hangul[mxCharSet_Script_Hangul] = {
	0x001100, 0x001200, 0x00302e, 0x003030, 0x003131, 0x00318f, 0x003200, 0x00321f, 0x003260, 0x00327f, 0x00a960, 0x00a97d, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 
	0x00d7cb, 0x00d7fc, 0x00ffa0, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 
};
#define mxCharSet_Script_Hanifi_Rohingya 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hanifi_Rohingya[mxCharSet_Script_Hanifi_Rohingya] = {
	0x010d00, 0x010d28, 0x010d30, 0x010d3a, 
};
#define mxCharSet_Script_Hanunoo 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hanunoo[mxCharSet_Script_Hanunoo] = {
	0x001720, 0x001735, 
};
#define mxCharSet_Script_Hatran 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hatran[mxCharSet_Script_Hatran] = {
	0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x0108fb, 0x010900, 
};
#define mxCharSet_Script_Hebrew 18
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hebrew[mxCharSet_Script_Hebrew] = {
	0x000591, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x00fb1d, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 
	0x00fb46, 0x00fb50, 
};
#define mxCharSet_Script_Hiragana 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Hiragana[mxCharSet_Script_Hiragana] = {
	0x003041, 0x003097, 0x00309d, 0x0030a0, 0x01b001, 0x01b120, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 0x01f200, 0x01f201, 
};
#define mxCharSet_Script_Imperial_Aramaic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Imperial_Aramaic[mxCharSet_Script_Imperial_Aramaic] = {
	0x010840, 0x010856, 0x010857, 0x010860, 
};
#define mxCharSet_Script_Inherited 58
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Inherited[mxCharSet_Script_Inherited] = {
	0x000300, 0x000370, 0x000485, 0x000487, 0x00064b, 0x000656, 0x000670, 0x000671, 0x000951, 0x000955, 0x001ab0, 0x001acf, 0x001cd0, 0x001cd3, 0x001cd4, 0x001ce1, 
	0x001ce2, 0x001ce9, 0x001ced, 0x001cee, 0x001cf4, 0x001cf5, 0x001cf8, 0x001cfa, 0x001dc0, 0x001e00, 0x00200c, 0x00200e, 0x0020d0, 0x0020f1, 0x00302a, 0x00302e, 
	0x003099, 0x00309b, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe2e, 0x0101fd, 0x0101fe, 0x0102e0, 0x0102e1, 0x01133b, 0x01133c, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 
	0x01d167, 0x01d16a, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Script_Inscriptional_Pahlavi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Inscriptional_Pahlavi[mxCharSet_Script_Inscriptional_Pahlavi] = {
	0x010b60, 0x010b73, 0x010b78, 0x010b80, 
};
#define mxCharSet_Script_Inscriptional_Parthian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Inscriptional_Parthian[mxCharSet_Script_Inscriptional_Parthian] = {
	0x010b40, 0x010b56, 0x010b58, 0x010b60, 
};
#define mxCharSet_Script_Javanese 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Javanese[mxCharSet_Script_Javanese] = {
	0x00a980, 0x00a9ce, 0x00a9d0, 0x00a9da, 0x00a9de, 0x00a9e0, 
};
#define mxCharSet_Script_Kaithi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Kaithi[mxCharSet_Script_Kaithi] = {
	0x011080, 0x0110c3, 0x0110cd, 0x0110ce, 
};
#define mxCharSet_Script_Kannada 26
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Kannada[mxCharSet_Script_Kannada] = {
	0x000c80, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 
	0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 
};
#define mxCharSet_Script_Katakana 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Katakana[mxCharSet_Script_Katakana] = {
	0x0030a1, 0x0030fb, 0x0030fd, 0x003100, 0x0031f0, 0x003200, 0x0032d0, 0x0032ff, 0x003300, 0x003358, 0x00ff66, 0x00ff70, 0x00ff71, 0x00ff9e, 0x01aff0, 0x01aff4, 
	0x01aff5, 0x01affc, 0x01affd, 0x01afff, 0x01b000, 0x01b001, 0x01b120, 0x01b123, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 
};
#define mxCharSet_Script_Kawi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Kawi[mxCharSet_Script_Kawi] = {
	0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f5a, 
};
#define mxCharSet_Script_Kayah_Li 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Kayah_Li[mxCharSet_Script_Kayah_Li] = {
	0x00a900, 0x00a92e, 0x00a92f, 0x00a930, 
};
#define mxCharSet_Script_Kharoshthi 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Kharoshthi[mxCharSet_Script_Kharoshthi] = {
	0x010a00, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a49, 0x010a50, 0x010a59, 
};
#define mxCharSet_Script_Khitan_Small_Script 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Khitan_Small_Script[mxCharSet_Script_Khitan_Small_Script] = {
	0x016fe4, 0x016fe5, 0x018b00, 0x018cd6, 
};
#define mxCharSet_Script_Khmer 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Khmer[mxCharSet_Script_Khmer] = {
	0x001780, 0x0017de, 0x0017e0, 0x0017ea, 0x0017f0, 0x0017fa, 0x0019e0, 0x001a00, 
};
#define mxCharSet_Script_Khojki 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Khojki[mxCharSet_Script_Khojki] = {
	0x011200, 0x011212, 0x011213, 0x011242, 
};
#define mxCharSet_Script_Khudawadi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Khudawadi[mxCharSet_Script_Khudawadi] = {
	0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 
};
#define mxCharSet_Script_Lao 22
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Lao[mxCharSet_Script_Lao] = {
	0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 
	0x000ec8, 0x000ecf, 0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 
};
#define mxCharSet_Script_Latin 78
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Latin[mxCharSet_Script_Latin] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002b9, 0x0002e0, 0x0002e5, 
	0x001d00, 0x001d26, 0x001d2c, 0x001d5d, 0x001d62, 0x001d66, 0x001d6b, 0x001d78, 0x001d79, 0x001dbf, 0x001e00, 0x001f00, 0x002071, 0x002072, 0x00207f, 0x002080, 
	0x002090, 0x00209d, 0x00212a, 0x00212c, 0x002132, 0x002133, 0x00214e, 0x00214f, 0x002160, 0x002189, 0x002c60, 0x002c80, 0x00a722, 0x00a788, 0x00a78b, 0x00a7cb, 
	0x00a7d0, 0x00a7d2, 0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a800, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab65, 0x00ab66, 0x00ab6a, 0x00fb00, 0x00fb07, 
	0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 
};
#define mxCharSet_Script_Lepcha 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Lepcha[mxCharSet_Script_Lepcha] = {
	0x001c00, 0x001c38, 0x001c3b, 0x001c4a, 0x001c4d, 0x001c50, 
};
#define mxCharSet_Script_Limbu 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Limbu[mxCharSet_Script_Limbu] = {
	0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x00193c, 0x001940, 0x001941, 0x001944, 0x001950, 
};
#define mxCharSet_Script_Linear_A 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Linear_A[mxCharSet_Script_Linear_A] = {
	0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 
};
#define mxCharSet_Script_Linear_B 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Linear_B[mxCharSet_Script_Linear_B] = {
	0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 
};
#define mxCharSet_Script_Lisu 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Lisu[mxCharSet_Script_Lisu] = {
	0x00a4d0, 0x00a500, 0x011fb0, 0x011fb1, 
};
#define mxCharSet_Script_Lycian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Lycian[mxCharSet_Script_Lycian] = {
	0x010280, 0x01029d, 
};
#define mxCharSet_Script_Lydian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Lydian[mxCharSet_Script_Lydian] = {
	0x010920, 0x01093a, 0x01093f, 0x010940, 
};
#define mxCharSet_Script_Mahajani 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Mahajani[mxCharSet_Script_Mahajani] = {
	0x011150, 0x011177, 
};
#define mxCharSet_Script_Makasar 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Makasar[mxCharSet_Script_Makasar] = {
	0x011ee0, 0x011ef9, 
};
#define mxCharSet_Script_Malayalam 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Malayalam[mxCharSet_Script_Malayalam] = {
	0x000d00, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d50, 0x000d54, 0x000d64, 0x000d66, 0x000d80, 
};
#define mxCharSet_Script_Mandaic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Mandaic[mxCharSet_Script_Mandaic] = {
	0x000840, 0x00085c, 0x00085e, 0x00085f, 
};
#define mxCharSet_Script_Manichaean 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Manichaean[mxCharSet_Script_Manichaean] = {
	0x010ac0, 0x010ae7, 0x010aeb, 0x010af7, 
};
#define mxCharSet_Script_Marchen 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Marchen[mxCharSet_Script_Marchen] = {
	0x011c70, 0x011c90, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 
};
#define mxCharSet_Script_Masaram_Gondi 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Masaram_Gondi[mxCharSet_Script_Masaram_Gondi] = {
	0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 
};
#define mxCharSet_Script_Medefaidrin 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Medefaidrin[mxCharSet_Script_Medefaidrin] = {
	0x016e40, 0x016e9b, 
};
#define mxCharSet_Script_Meetei_Mayek 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Meetei_Mayek[mxCharSet_Script_Meetei_Mayek] = {
	0x00aae0, 0x00aaf7, 0x00abc0, 0x00abee, 0x00abf0, 0x00abfa, 
};
#define mxCharSet_Script_Mende_Kikakui 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Mende_Kikakui[mxCharSet_Script_Mende_Kikakui] = {
	0x01e800, 0x01e8c5, 0x01e8c7, 0x01e8d7, 
};
#define mxCharSet_Script_Meroitic_Cursive 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Meroitic_Cursive[mxCharSet_Script_Meroitic_Cursive] = {
	0x0109a0, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a00, 
};
#define mxCharSet_Script_Meroitic_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Meroitic_Hieroglyphs[mxCharSet_Script_Meroitic_Hieroglyphs] = {
	0x010980, 0x0109a0, 
};
#define mxCharSet_Script_Miao 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Miao[mxCharSet_Script_Miao] = {
	0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 
};
#define mxCharSet_Script_Modi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Modi[mxCharSet_Script_Modi] = {
	0x011600, 0x011645, 0x011650, 0x01165a, 
};
#define mxCharSet_Script_Mongolian 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Mongolian[mxCharSet_Script_Mongolian] = {
	0x001800, 0x001802, 0x001804, 0x001805, 0x001806, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x011660, 0x01166d, 
};
#define mxCharSet_Script_Mro 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Mro[mxCharSet_Script_Mro] = {
	0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a6e, 0x016a70, 
};
#define mxCharSet_Script_Multani 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Multani[mxCharSet_Script_Multani] = {
	0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 
};
#define mxCharSet_Script_Myanmar 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Myanmar[mxCharSet_Script_Myanmar] = {
	0x001000, 0x0010a0, 0x00a9e0, 0x00a9ff, 0x00aa60, 0x00aa80, 
};
#define mxCharSet_Script_Nabataean 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nabataean[mxCharSet_Script_Nabataean] = {
	0x010880, 0x01089f, 0x0108a7, 0x0108b0, 
};
#define mxCharSet_Script_Nag_Mundari 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nag_Mundari[mxCharSet_Script_Nag_Mundari] = {
	0x01e4d0, 0x01e4fa, 
};
#define mxCharSet_Script_Nandinagari 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nandinagari[mxCharSet_Script_Nandinagari] = {
	0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 0x0119da, 0x0119e5, 
};
#define mxCharSet_Script_New_Tai_Lue 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_New_Tai_Lue[mxCharSet_Script_New_Tai_Lue] = {
	0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x0019e0, 
};
#define mxCharSet_Script_Newa 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Newa[mxCharSet_Script_Newa] = {
	0x011400, 0x01145c, 0x01145d, 0x011462, 
};
#define mxCharSet_Script_Nko 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nko[mxCharSet_Script_Nko] = {
	0x0007c0, 0x0007fb, 0x0007fd, 0x000800, 
};
#define mxCharSet_Script_Nushu 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nushu[mxCharSet_Script_Nushu] = {
	0x016fe1, 0x016fe2, 0x01b170, 0x01b2fc, 
};
#define mxCharSet_Script_Nyiakeng_Puachue_Hmong 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Nyiakeng_Puachue_Hmong[mxCharSet_Script_Nyiakeng_Puachue_Hmong] = {
	0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 
};
#define mxCharSet_Script_Ogham 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Ogham[mxCharSet_Script_Ogham] = {
	0x001680, 0x00169d, 
};
#define mxCharSet_Script_Ol_Chiki 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Ol_Chiki[mxCharSet_Script_Ol_Chiki] = {
	0x001c50, 0x001c80, 
};
#define mxCharSet_Script_Old_Hungarian 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Hungarian[mxCharSet_Script_Old_Hungarian] = {
	0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010cfa, 0x010d00, 
};
#define mxCharSet_Script_Old_Italic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Italic[mxCharSet_Script_Old_Italic] = {
	0x010300, 0x010324, 0x01032d, 0x010330, 
};
#define mxCharSet_Script_Old_North_Arabian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_North_Arabian[mxCharSet_Script_Old_North_Arabian] = {
	0x010a80, 0x010aa0, 
};
#define mxCharSet_Script_Old_Permic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Permic[mxCharSet_Script_Old_Permic] = {
	0x010350, 0x01037b, 
};
#define mxCharSet_Script_Old_Persian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Persian[mxCharSet_Script_Old_Persian] = {
	0x0103a0, 0x0103c4, 0x0103c8, 0x0103d6, 
};
#define mxCharSet_Script_Old_Sogdian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Sogdian[mxCharSet_Script_Old_Sogdian] = {
	0x010f00, 0x010f28, 
};
#define mxCharSet_Script_Old_South_Arabian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_South_Arabian[mxCharSet_Script_Old_South_Arabian] = {
	0x010a60, 0x010a80, 
};
#define mxCharSet_Script_Old_Turkic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Turkic[mxCharSet_Script_Old_Turkic] = {
	0x010c00, 0x010c49, 
};
#define mxCharSet_Script_Old_Uyghur 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Old_Uyghur[mxCharSet_Script_Old_Uyghur] = {
	0x010f70, 0x010f8a, 
};
#define mxCharSet_Script_Oriya 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Oriya[mxCharSet_Script_Oriya] = {
	0x000b01, 0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 
	0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 0x000b78, 
};
#define mxCharSet_Script_Osage 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Osage[mxCharSet_Script_Osage] = {
	0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 
};
#define mxCharSet_Script_Osmanya 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Osmanya[mxCharSet_Script_Osmanya] = {
	0x010480, 0x01049e, 0x0104a0, 0x0104aa, 
};
#define mxCharSet_Script_Pahawh_Hmong 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Pahawh_Hmong[mxCharSet_Script_Pahawh_Hmong] = {
	0x016b00, 0x016b46, 0x016b50, 0x016b5a, 0x016b5b, 0x016b62, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 
};
#define mxCharSet_Script_Palmyrene 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Palmyrene[mxCharSet_Script_Palmyrene] = {
	0x010860, 0x010880, 
};
#define mxCharSet_Script_Pau_Cin_Hau 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Pau_Cin_Hau[mxCharSet_Script_Pau_Cin_Hau] = {
	0x011ac0, 0x011af9, 
};
#define mxCharSet_Script_Phags_Pa 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Phags_Pa[mxCharSet_Script_Phags_Pa] = {
	0x00a840, 0x00a878, 
};
#define mxCharSet_Script_Phoenician 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Phoenician[mxCharSet_Script_Phoenician] = {
	0x010900, 0x01091c, 0x01091f, 0x010920, 
};
#define mxCharSet_Script_Psalter_Pahlavi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Psalter_Pahlavi[mxCharSet_Script_Psalter_Pahlavi] = {
	0x010b80, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 
};
#define mxCharSet_Script_Rejang 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Rejang[mxCharSet_Script_Rejang] = {
	0x00a930, 0x00a954, 0x00a95f, 0x00a960, 
};
#define mxCharSet_Script_Runic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Runic[mxCharSet_Script_Runic] = {
	0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 
};
#define mxCharSet_Script_Samaritan 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Samaritan[mxCharSet_Script_Samaritan] = {
	0x000800, 0x00082e, 0x000830, 0x00083f, 
};
#define mxCharSet_Script_Saurashtra 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Saurashtra[mxCharSet_Script_Saurashtra] = {
	0x00a880, 0x00a8c6, 0x00a8ce, 0x00a8da, 
};
#define mxCharSet_Script_Sharada 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Sharada[mxCharSet_Script_Sharada] = {
	0x011180, 0x0111e0, 
};
#define mxCharSet_Script_Shavian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Shavian[mxCharSet_Script_Shavian] = {
	0x010450, 0x010480, 
};
#define mxCharSet_Script_Siddham 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Siddham[mxCharSet_Script_Siddham] = {
	0x011580, 0x0115b6, 0x0115b8, 0x0115de, 
};
#define mxCharSet_Script_SignWriting 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_SignWriting[mxCharSet_Script_SignWriting] = {
	0x01d800, 0x01da8c, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 
};
#define mxCharSet_Script_Sinhala 26
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Sinhala[mxCharSet_Script_Sinhala] = {
	0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 0x000dcf, 0x000dd5, 
	0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df5, 0x0111e1, 0x0111f5, 
};
#define mxCharSet_Script_Sogdian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Sogdian[mxCharSet_Script_Sogdian] = {
	0x010f30, 0x010f5a, 
};
#define mxCharSet_Script_Sora_Sompeng 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Sora_Sompeng[mxCharSet_Script_Sora_Sompeng] = {
	0x0110d0, 0x0110e9, 0x0110f0, 0x0110fa, 
};
#define mxCharSet_Script_Soyombo 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Soyombo[mxCharSet_Script_Soyombo] = {
	0x011a50, 0x011aa3, 
};
#define mxCharSet_Script_Sundanese 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Sundanese[mxCharSet_Script_Sundanese] = {
	0x001b80, 0x001bc0, 0x001cc0, 0x001cc8, 
};
#define mxCharSet_Script_Syloti_Nagri 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Syloti_Nagri[mxCharSet_Script_Syloti_Nagri] = {
	0x00a800, 0x00a82d, 
};
#define mxCharSet_Script_Syriac 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Syriac[mxCharSet_Script_Syriac] = {
	0x000700, 0x00070e, 0x00070f, 0x00074b, 0x00074d, 0x000750, 0x000860, 0x00086b, 
};
#define mxCharSet_Script_Tagalog 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tagalog[mxCharSet_Script_Tagalog] = {
	0x001700, 0x001716, 0x00171f, 0x001720, 
};
#define mxCharSet_Script_Tagbanwa 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tagbanwa[mxCharSet_Script_Tagbanwa] = {
	0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 
};
#define mxCharSet_Script_Tai_Le 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tai_Le[mxCharSet_Script_Tai_Le] = {
	0x001950, 0x00196e, 0x001970, 0x001975, 
};
#define mxCharSet_Script_Tai_Tham 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tai_Tham[mxCharSet_Script_Tai_Tham] = {
	0x001a20, 0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 
};
#define mxCharSet_Script_Tai_Viet 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tai_Viet[mxCharSet_Script_Tai_Viet] = {
	0x00aa80, 0x00aac3, 0x00aadb, 0x00aae0, 
};
#define mxCharSet_Script_Takri 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Takri[mxCharSet_Script_Takri] = {
	0x011680, 0x0116ba, 0x0116c0, 0x0116ca, 
};
#define mxCharSet_Script_Tamil 36
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tamil[mxCharSet_Script_Tamil] = {
	0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 
	0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 0x000bd7, 0x000bd8, 0x000be6, 0x000bfb, 
	0x011fc0, 0x011ff2, 0x011fff, 0x012000, 
};
#define mxCharSet_Script_Tangsa 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tangsa[mxCharSet_Script_Tangsa] = {
	0x016a70, 0x016abf, 0x016ac0, 0x016aca, 
};
#define mxCharSet_Script_Tangut 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tangut[mxCharSet_Script_Tangut] = {
	0x016fe0, 0x016fe1, 0x017000, 0x0187f8, 0x018800, 0x018b00, 0x018d00, 0x018d09, 
};
#define mxCharSet_Script_Telugu 26
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Telugu[mxCharSet_Script_Telugu] = {
	0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 
	0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 0x000c66, 0x000c70, 0x000c77, 0x000c80, 
};
#define mxCharSet_Script_Thaana 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Thaana[mxCharSet_Script_Thaana] = {
	0x000780, 0x0007b2, 
};
#define mxCharSet_Script_Thai 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Thai[mxCharSet_Script_Thai] = {
	0x000e01, 0x000e3b, 0x000e40, 0x000e5c, 
};
#define mxCharSet_Script_Tibetan 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tibetan[mxCharSet_Script_Tibetan] = {
	0x000f00, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f98, 0x000f99, 0x000fbd, 0x000fbe, 0x000fcd, 0x000fce, 0x000fd5, 0x000fd9, 0x000fdb, 
};
#define mxCharSet_Script_Tifinagh 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tifinagh[mxCharSet_Script_Tifinagh] = {
	0x002d30, 0x002d68, 0x002d6f, 0x002d71, 0x002d7f, 0x002d80, 
};
#define mxCharSet_Script_Tirhuta 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Tirhuta[mxCharSet_Script_Tirhuta] = {
	0x011480, 0x0114c8, 0x0114d0, 0x0114da, 
};
#define mxCharSet_Script_Toto 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Toto[mxCharSet_Script_Toto] = {
	0x01e290, 0x01e2af, 
};
#define mxCharSet_Script_Ugaritic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Ugaritic[mxCharSet_Script_Ugaritic] = {
	0x010380, 0x01039e, 0x01039f, 0x0103a0, 
};
#define mxCharSet_Script_Vai 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Vai[mxCharSet_Script_Vai] = {
	0x00a500, 0x00a62c, 
};
#define mxCharSet_Script_Vithkuqi 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Vithkuqi[mxCharSet_Script_Vithkuqi] = {
	0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 
};
#define mxCharSet_Script_Wancho 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Wancho[mxCharSet_Script_Wancho] = {
	0x01e2c0, 0x01e2fa, 0x01e2ff, 0x01e300, 
};
#define mxCharSet_Script_Warang_Citi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Warang_Citi[mxCharSet_Script_Warang_Citi] = {
	0x0118a0, 0x0118f3, 0x0118ff, 0x011900, 
};
#define mxCharSet_Script_Yezidi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Yezidi[mxCharSet_Script_Yezidi] = {
	0x010e80, 0x010eaa, 0x010eab, 0x010eae, 0x010eb0, 0x010eb2, 
};
#define mxCharSet_Script_Yi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Yi[mxCharSet_Script_Yi] = {
	0x00a000, 0x00a48d, 0x00a490, 0x00a4c7, 
};
#define mxCharSet_Script_Zanabazar_Square 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Zanabazar_Square[mxCharSet_Script_Zanabazar_Square] = {
	0x011a00, 0x011a48, 
};
#define mxCharSet_Script_Extensions_Adlam 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Adlam[mxCharSet_Script_Extensions_Adlam] = {
	0x00061f, 0x000620, 0x000640, 0x000641, 0x01e900, 0x01e94c, 0x01e950, 0x01e95a, 0x01e95e, 0x01e960, 
};
#define mxCharSet_Script_Extensions_Ahom 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Ahom[mxCharSet_Script_Extensions_Ahom] = {
	0x011700, 0x01171b, 0x01171d, 0x01172c, 0x011730, 0x011747, 
};
#define mxCharSet_Script_Extensions_Anatolian_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Anatolian_Hieroglyphs[mxCharSet_Script_Extensions_Anatolian_Hieroglyphs] = {
	0x014400, 0x014647, 
};
#define mxCharSet_Script_Extensions_Arabic 104
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Arabic[mxCharSet_Script_Extensions_Arabic] = {
	0x000600, 0x000605, 0x000606, 0x0006dd, 0x0006de, 0x000700, 0x000750, 0x000780, 0x000870, 0x00088f, 0x000890, 0x000892, 0x000898, 0x0008e2, 0x0008e3, 0x000900, 
	0x00fb50, 0x00fbc3, 0x00fbd3, 0x00fd90, 0x00fd92, 0x00fdc8, 0x00fdcf, 0x00fdd0, 0x00fdf0, 0x00fe00, 0x00fe70, 0x00fe75, 0x00fe76, 0x00fefd, 0x0102e0, 0x0102fc, 
	0x010e60, 0x010e7f, 0x010efd, 0x010f00, 0x01ee00, 0x01ee04, 0x01ee05, 0x01ee20, 0x01ee21, 0x01ee23, 0x01ee24, 0x01ee25, 0x01ee27, 0x01ee28, 0x01ee29, 0x01ee33, 
	0x01ee34, 0x01ee38, 0x01ee39, 0x01ee3a, 0x01ee3b, 0x01ee3c, 0x01ee42, 0x01ee43, 0x01ee47, 0x01ee48, 0x01ee49, 0x01ee4a, 0x01ee4b, 0x01ee4c, 0x01ee4d, 0x01ee50, 
	0x01ee51, 0x01ee53, 0x01ee54, 0x01ee55, 0x01ee57, 0x01ee58, 0x01ee59, 0x01ee5a, 0x01ee5b, 0x01ee5c, 0x01ee5d, 0x01ee5e, 0x01ee5f, 0x01ee60, 0x01ee61, 0x01ee63, 
	0x01ee64, 0x01ee65, 0x01ee67, 0x01ee6b, 0x01ee6c, 0x01ee73, 0x01ee74, 0x01ee78, 0x01ee79, 0x01ee7d, 0x01ee7e, 0x01ee7f, 0x01ee80, 0x01ee8a, 0x01ee8b, 0x01ee9c, 
	0x01eea1, 0x01eea4, 0x01eea5, 0x01eeaa, 0x01eeab, 0x01eebc, 0x01eef0, 0x01eef2, 
};
#define mxCharSet_Script_Extensions_Armenian 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Armenian[mxCharSet_Script_Extensions_Armenian] = {
	0x000531, 0x000557, 0x000559, 0x00058b, 0x00058d, 0x000590, 0x00fb13, 0x00fb18, 
};
#define mxCharSet_Script_Extensions_Avestan 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Avestan[mxCharSet_Script_Extensions_Avestan] = {
	0x010b00, 0x010b36, 0x010b39, 0x010b40, 
};
#define mxCharSet_Script_Extensions_Balinese 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Balinese[mxCharSet_Script_Extensions_Balinese] = {
	0x001b00, 0x001b4d, 0x001b50, 0x001b7f, 
};
#define mxCharSet_Script_Extensions_Bamum 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Bamum[mxCharSet_Script_Extensions_Bamum] = {
	0x00a6a0, 0x00a6f8, 0x016800, 0x016a39, 
};
#define mxCharSet_Script_Extensions_Bassa_Vah 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Bassa_Vah[mxCharSet_Script_Extensions_Bassa_Vah] = {
	0x016ad0, 0x016aee, 0x016af0, 0x016af6, 
};
#define mxCharSet_Script_Extensions_Batak 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Batak[mxCharSet_Script_Extensions_Batak] = {
	0x001bc0, 0x001bf4, 0x001bfc, 0x001c00, 
};
#define mxCharSet_Script_Extensions_Bengali 52
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Bengali[mxCharSet_Script_Extensions_Bengali] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000980, 0x000984, 0x000985, 0x00098d, 0x00098f, 0x000991, 0x000993, 0x0009a9, 0x0009aa, 0x0009b1, 0x0009b2, 0x0009b3, 
	0x0009b6, 0x0009ba, 0x0009bc, 0x0009c5, 0x0009c7, 0x0009c9, 0x0009cb, 0x0009cf, 0x0009d7, 0x0009d8, 0x0009dc, 0x0009de, 0x0009df, 0x0009e4, 0x0009e6, 0x0009ff, 
	0x001cd0, 0x001cd1, 0x001cd2, 0x001cd3, 0x001cd5, 0x001cd7, 0x001cd8, 0x001cd9, 0x001ce1, 0x001ce2, 0x001cea, 0x001ceb, 0x001ced, 0x001cee, 0x001cf2, 0x001cf3, 
	0x001cf5, 0x001cf8, 0x00a8f1, 0x00a8f2, 
};
#define mxCharSet_Script_Extensions_Bhaiksuki 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Bhaiksuki[mxCharSet_Script_Extensions_Bhaiksuki] = {
	0x011c00, 0x011c09, 0x011c0a, 0x011c37, 0x011c38, 0x011c46, 0x011c50, 0x011c6d, 
};
#define mxCharSet_Script_Extensions_Bopomofo 24
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Bopomofo[mxCharSet_Script_Extensions_Bopomofo] = {
	0x0002ea, 0x0002ec, 0x003001, 0x003004, 0x003008, 0x003012, 0x003013, 0x003020, 0x00302a, 0x00302e, 0x003030, 0x003031, 0x003037, 0x003038, 0x0030fb, 0x0030fc, 
	0x003105, 0x003130, 0x0031a0, 0x0031c0, 0x00fe45, 0x00fe47, 0x00ff61, 0x00ff66, 
};
#define mxCharSet_Script_Extensions_Brahmi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Brahmi[mxCharSet_Script_Extensions_Brahmi] = {
	0x011000, 0x01104e, 0x011052, 0x011076, 0x01107f, 0x011080, 
};
#define mxCharSet_Script_Extensions_Braille 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Braille[mxCharSet_Script_Extensions_Braille] = {
	0x002800, 0x002900, 
};
#define mxCharSet_Script_Extensions_Buginese 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Buginese[mxCharSet_Script_Extensions_Buginese] = {
	0x001a00, 0x001a1c, 0x001a1e, 0x001a20, 0x00a9cf, 0x00a9d0, 
};
#define mxCharSet_Script_Extensions_Buhid 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Buhid[mxCharSet_Script_Extensions_Buhid] = {
	0x001735, 0x001737, 0x001740, 0x001754, 
};
#define mxCharSet_Script_Extensions_Canadian_Aboriginal 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Canadian_Aboriginal[mxCharSet_Script_Extensions_Canadian_Aboriginal] = {
	0x001400, 0x001680, 0x0018b0, 0x0018f6, 0x011ab0, 0x011ac0, 
};
#define mxCharSet_Script_Extensions_Carian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Carian[mxCharSet_Script_Extensions_Carian] = {
	0x0102a0, 0x0102d1, 
};
#define mxCharSet_Script_Extensions_Caucasian_Albanian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Caucasian_Albanian[mxCharSet_Script_Extensions_Caucasian_Albanian] = {
	0x010530, 0x010564, 0x01056f, 0x010570, 
};
#define mxCharSet_Script_Extensions_Chakma 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Chakma[mxCharSet_Script_Extensions_Chakma] = {
	0x0009e6, 0x0009f0, 0x001040, 0x00104a, 0x011100, 0x011135, 0x011136, 0x011148, 
};
#define mxCharSet_Script_Extensions_Cham 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cham[mxCharSet_Script_Extensions_Cham] = {
	0x00aa00, 0x00aa37, 0x00aa40, 0x00aa4e, 0x00aa50, 0x00aa5a, 0x00aa5c, 0x00aa60, 
};
#define mxCharSet_Script_Extensions_Cherokee 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cherokee[mxCharSet_Script_Extensions_Cherokee] = {
	0x0013a0, 0x0013f6, 0x0013f8, 0x0013fe, 0x00ab70, 0x00abc0, 
};
#define mxCharSet_Script_Extensions_Chorasmian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Chorasmian[mxCharSet_Script_Extensions_Chorasmian] = {
	0x010fb0, 0x010fcc, 
};
#define mxCharSet_Script_Extensions_Common 294
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Common[mxCharSet_Script_Extensions_Common] = {
	0x000000, 0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002b9, 0x0002e0, 
	0x0002e5, 0x0002ea, 0x0002ec, 0x000300, 0x000374, 0x000375, 0x00037e, 0x00037f, 0x000385, 0x000386, 0x000387, 0x000388, 0x000605, 0x000606, 0x0006dd, 0x0006de, 
	0x0008e2, 0x0008e3, 0x000e3f, 0x000e40, 0x000fd5, 0x000fd9, 0x0016eb, 0x0016ee, 0x002000, 0x00200c, 0x00200e, 0x00202f, 0x002030, 0x002065, 0x002066, 0x002071, 
	0x002074, 0x00207f, 0x002080, 0x00208f, 0x0020a0, 0x0020c1, 0x002100, 0x002126, 0x002127, 0x00212a, 0x00212c, 0x002132, 0x002133, 0x00214e, 0x00214f, 0x002160, 
	0x002189, 0x00218c, 0x002190, 0x002427, 0x002440, 0x00244b, 0x002460, 0x002800, 0x002900, 0x002b74, 0x002b76, 0x002b96, 0x002b97, 0x002c00, 0x002e00, 0x002e43, 
	0x002e44, 0x002e5e, 0x002ff0, 0x002ffc, 0x003000, 0x003001, 0x003004, 0x003005, 0x003012, 0x003013, 0x003020, 0x003021, 0x003036, 0x003037, 0x003248, 0x003260, 
	0x00327f, 0x003280, 0x0032b1, 0x0032c0, 0x0032cc, 0x0032d0, 0x003371, 0x00337b, 0x003380, 0x0033e0, 0x0033ff, 0x003400, 0x004dc0, 0x004e00, 0x00a708, 0x00a722, 
	0x00a788, 0x00a78b, 0x00ab5b, 0x00ab5c, 0x00ab6a, 0x00ab6c, 0x00fe10, 0x00fe1a, 0x00fe30, 0x00fe45, 0x00fe47, 0x00fe53, 0x00fe54, 0x00fe67, 0x00fe68, 0x00fe6c, 
	0x00feff, 0x00ff00, 0x00ff01, 0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x00ff61, 0x00ffe0, 0x00ffe7, 0x00ffe8, 0x00ffef, 0x00fff9, 0x00fffe, 0x010190, 0x01019d, 
	0x0101d0, 0x0101fd, 0x01cf50, 0x01cfc4, 0x01d000, 0x01d0f6, 0x01d100, 0x01d127, 0x01d129, 0x01d167, 0x01d16a, 0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 
	0x01d1ae, 0x01d1eb, 0x01d2c0, 0x01d2d4, 0x01d2e0, 0x01d2f4, 0x01d300, 0x01d357, 0x01d372, 0x01d379, 0x01d400, 0x01d455, 0x01d456, 0x01d49d, 0x01d49e, 0x01d4a0, 
	0x01d4a2, 0x01d4a3, 0x01d4a5, 0x01d4a7, 0x01d4a9, 0x01d4ad, 0x01d4ae, 0x01d4ba, 0x01d4bb, 0x01d4bc, 0x01d4bd, 0x01d4c4, 0x01d4c5, 0x01d506, 0x01d507, 0x01d50b, 
	0x01d50d, 0x01d515, 0x01d516, 0x01d51d, 0x01d51e, 0x01d53a, 0x01d53b, 0x01d53f, 0x01d540, 0x01d545, 0x01d546, 0x01d547, 0x01d54a, 0x01d551, 0x01d552, 0x01d6a6, 
	0x01d6a8, 0x01d7cc, 0x01d7ce, 0x01d800, 0x01ec71, 0x01ecb5, 0x01ed01, 0x01ed3e, 0x01f000, 0x01f02c, 0x01f030, 0x01f094, 0x01f0a0, 0x01f0af, 0x01f0b1, 0x01f0c0, 
	0x01f0c1, 0x01f0d0, 0x01f0d1, 0x01f0f6, 0x01f100, 0x01f1ae, 0x01f1e6, 0x01f200, 0x01f201, 0x01f203, 0x01f210, 0x01f23c, 0x01f240, 0x01f249, 0x01f260, 0x01f266, 
	0x01f300, 0x01f6d8, 0x01f6dc, 0x01f6ed, 0x01f6f0, 0x01f6fd, 0x01f700, 0x01f777, 0x01f77b, 0x01f7da, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f800, 0x01f80c, 
	0x01f810, 0x01f848, 0x01f850, 0x01f85a, 0x01f860, 0x01f888, 0x01f890, 0x01f8ae, 0x01f8b0, 0x01f8b2, 0x01f900, 0x01fa54, 0x01fa60, 0x01fa6e, 0x01fa70, 0x01fa7d, 
	0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 0x01fb00, 0x01fb93, 0x01fb94, 0x01fbcb, 
	0x01fbf0, 0x01fbfa, 0x0e0001, 0x0e0002, 0x0e0020, 0x0e0080, 
};
#define mxCharSet_Script_Extensions_Coptic 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Coptic[mxCharSet_Script_Extensions_Coptic] = {
	0x0003e2, 0x0003f0, 0x002c80, 0x002cf4, 0x002cf9, 0x002d00, 0x0102e0, 0x0102fc, 
};
#define mxCharSet_Script_Extensions_Cuneiform 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cuneiform[mxCharSet_Script_Extensions_Cuneiform] = {
	0x012000, 0x01239a, 0x012400, 0x01246f, 0x012470, 0x012475, 0x012480, 0x012544, 
};
#define mxCharSet_Script_Extensions_Cypriot 18
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cypriot[mxCharSet_Script_Extensions_Cypriot] = {
	0x010100, 0x010103, 0x010107, 0x010134, 0x010137, 0x010140, 0x010800, 0x010806, 0x010808, 0x010809, 0x01080a, 0x010836, 0x010837, 0x010839, 0x01083c, 0x01083d, 
	0x01083f, 0x010840, 
};
#define mxCharSet_Script_Extensions_Cypro_Minoan 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cypro_Minoan[mxCharSet_Script_Extensions_Cypro_Minoan] = {
	0x010100, 0x010102, 0x012f90, 0x012ff3, 
};
#define mxCharSet_Script_Extensions_Cyrillic 22
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Cyrillic[mxCharSet_Script_Extensions_Cyrillic] = {
	0x000400, 0x000530, 0x001c80, 0x001c89, 0x001d2b, 0x001d2c, 0x001d78, 0x001d79, 0x001df8, 0x001df9, 0x002de0, 0x002e00, 0x002e43, 0x002e44, 0x00a640, 0x00a6a0, 
	0x00fe2e, 0x00fe30, 0x01e030, 0x01e06e, 0x01e08f, 0x01e090, 
};
#define mxCharSet_Script_Extensions_Deseret 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Deseret[mxCharSet_Script_Extensions_Deseret] = {
	0x010400, 0x010450, 
};
#define mxCharSet_Script_Extensions_Devanagari 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Devanagari[mxCharSet_Script_Extensions_Devanagari] = {
	0x000900, 0x000953, 0x000955, 0x000980, 0x001cd0, 0x001cf7, 0x001cf8, 0x001cfa, 0x0020f0, 0x0020f1, 0x00a830, 0x00a83a, 0x00a8e0, 0x00a900, 0x011b00, 0x011b0a, 
};
#define mxCharSet_Script_Extensions_Dives_Akuru 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Dives_Akuru[mxCharSet_Script_Extensions_Dives_Akuru] = {
	0x011900, 0x011907, 0x011909, 0x01190a, 0x01190c, 0x011914, 0x011915, 0x011917, 0x011918, 0x011936, 0x011937, 0x011939, 0x01193b, 0x011947, 0x011950, 0x01195a, 
};
#define mxCharSet_Script_Extensions_Dogra 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Dogra[mxCharSet_Script_Extensions_Dogra] = {
	0x000964, 0x000970, 0x00a830, 0x00a83a, 0x011800, 0x01183c, 
};
#define mxCharSet_Script_Extensions_Duployan 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Duployan[mxCharSet_Script_Extensions_Duployan] = {
	0x01bc00, 0x01bc6b, 0x01bc70, 0x01bc7d, 0x01bc80, 0x01bc89, 0x01bc90, 0x01bc9a, 0x01bc9c, 0x01bca4, 
};
#define mxCharSet_Script_Extensions_Egyptian_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Egyptian_Hieroglyphs[mxCharSet_Script_Extensions_Egyptian_Hieroglyphs] = {
	0x013000, 0x013456, 
};
#define mxCharSet_Script_Extensions_Elbasan 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Elbasan[mxCharSet_Script_Extensions_Elbasan] = {
	0x010500, 0x010528, 
};
#define mxCharSet_Script_Extensions_Elymaic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Elymaic[mxCharSet_Script_Extensions_Elymaic] = {
	0x010fe0, 0x010ff7, 
};
#define mxCharSet_Script_Extensions_Ethiopic 72
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Ethiopic[mxCharSet_Script_Extensions_Ethiopic] = {
	0x001200, 0x001249, 0x00124a, 0x00124e, 0x001250, 0x001257, 0x001258, 0x001259, 0x00125a, 0x00125e, 0x001260, 0x001289, 0x00128a, 0x00128e, 0x001290, 0x0012b1, 
	0x0012b2, 0x0012b6, 0x0012b8, 0x0012bf, 0x0012c0, 0x0012c1, 0x0012c2, 0x0012c6, 0x0012c8, 0x0012d7, 0x0012d8, 0x001311, 0x001312, 0x001316, 0x001318, 0x00135b, 
	0x00135d, 0x00137d, 0x001380, 0x00139a, 0x002d80, 0x002d97, 0x002da0, 0x002da7, 0x002da8, 0x002daf, 0x002db0, 0x002db7, 0x002db8, 0x002dbf, 0x002dc0, 0x002dc7, 
	0x002dc8, 0x002dcf, 0x002dd0, 0x002dd7, 0x002dd8, 0x002ddf, 0x00ab01, 0x00ab07, 0x00ab09, 0x00ab0f, 0x00ab11, 0x00ab17, 0x00ab20, 0x00ab27, 0x00ab28, 0x00ab2f, 
	0x01e7e0, 0x01e7e7, 0x01e7e8, 0x01e7ec, 0x01e7ed, 0x01e7ef, 0x01e7f0, 0x01e7ff, 
};
#define mxCharSet_Script_Extensions_Georgian 18
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Georgian[mxCharSet_Script_Extensions_Georgian] = {
	0x0010a0, 0x0010c6, 0x0010c7, 0x0010c8, 0x0010cd, 0x0010ce, 0x0010d0, 0x001100, 0x001c90, 0x001cbb, 0x001cbd, 0x001cc0, 0x002d00, 0x002d26, 0x002d27, 0x002d28, 
	0x002d2d, 0x002d2e, 
};
#define mxCharSet_Script_Extensions_Glagolitic 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Glagolitic[mxCharSet_Script_Extensions_Glagolitic] = {
	0x000484, 0x000485, 0x000487, 0x000488, 0x002c00, 0x002c60, 0x002e43, 0x002e44, 0x00a66f, 0x00a670, 0x01e000, 0x01e007, 0x01e008, 0x01e019, 0x01e01b, 0x01e022, 
	0x01e023, 0x01e025, 0x01e026, 0x01e02b, 
};
#define mxCharSet_Script_Extensions_Gothic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Gothic[mxCharSet_Script_Extensions_Gothic] = {
	0x010330, 0x01034b, 
};
#define mxCharSet_Script_Extensions_Grantha 50
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Grantha[mxCharSet_Script_Extensions_Grantha] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000be6, 0x000bf4, 0x001cd0, 0x001cd1, 0x001cd2, 0x001cd4, 0x001cf2, 0x001cf5, 0x001cf8, 0x001cfa, 0x0020f0, 0x0020f1, 
	0x011300, 0x011304, 0x011305, 0x01130d, 0x01130f, 0x011311, 0x011313, 0x011329, 0x01132a, 0x011331, 0x011332, 0x011334, 0x011335, 0x01133a, 0x01133b, 0x011345, 
	0x011347, 0x011349, 0x01134b, 0x01134e, 0x011350, 0x011351, 0x011357, 0x011358, 0x01135d, 0x011364, 0x011366, 0x01136d, 0x011370, 0x011375, 0x011fd0, 0x011fd2, 
	0x011fd3, 0x011fd4, 
};
#define mxCharSet_Script_Extensions_Greek 76
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Greek[mxCharSet_Script_Extensions_Greek] = {
	0x000342, 0x000343, 0x000345, 0x000346, 0x000370, 0x000374, 0x000375, 0x000378, 0x00037a, 0x00037e, 0x00037f, 0x000380, 0x000384, 0x000385, 0x000386, 0x000387, 
	0x000388, 0x00038b, 0x00038c, 0x00038d, 0x00038e, 0x0003a2, 0x0003a3, 0x0003e2, 0x0003f0, 0x000400, 0x001d26, 0x001d2b, 0x001d5d, 0x001d62, 0x001d66, 0x001d6b, 
	0x001dbf, 0x001dc2, 0x001f00, 0x001f16, 0x001f18, 0x001f1e, 0x001f20, 0x001f46, 0x001f48, 0x001f4e, 0x001f50, 0x001f58, 0x001f59, 0x001f5a, 0x001f5b, 0x001f5c, 
	0x001f5d, 0x001f5e, 0x001f5f, 0x001f7e, 0x001f80, 0x001fb5, 0x001fb6, 0x001fc5, 0x001fc6, 0x001fd4, 0x001fd6, 0x001fdc, 0x001fdd, 0x001ff0, 0x001ff2, 0x001ff5, 
	0x001ff6, 0x001fff, 0x002126, 0x002127, 0x00ab65, 0x00ab66, 0x010140, 0x01018f, 0x0101a0, 0x0101a1, 0x01d200, 0x01d246, 
};
#define mxCharSet_Script_Extensions_Gujarati 34
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Gujarati[mxCharSet_Script_Extensions_Gujarati] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000a81, 0x000a84, 0x000a85, 0x000a8e, 0x000a8f, 0x000a92, 0x000a93, 0x000aa9, 0x000aaa, 0x000ab1, 0x000ab2, 0x000ab4, 
	0x000ab5, 0x000aba, 0x000abc, 0x000ac6, 0x000ac7, 0x000aca, 0x000acb, 0x000ace, 0x000ad0, 0x000ad1, 0x000ae0, 0x000ae4, 0x000ae6, 0x000af2, 0x000af9, 0x000b00, 
	0x00a830, 0x00a83a, 
};
#define mxCharSet_Script_Extensions_Gunjala_Gondi 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Gunjala_Gondi[mxCharSet_Script_Extensions_Gunjala_Gondi] = {
	0x000964, 0x000966, 0x011d60, 0x011d66, 0x011d67, 0x011d69, 0x011d6a, 0x011d8f, 0x011d90, 0x011d92, 0x011d93, 0x011d99, 0x011da0, 0x011daa, 
};
#define mxCharSet_Script_Extensions_Gurmukhi 38
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Gurmukhi[mxCharSet_Script_Extensions_Gurmukhi] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000a01, 0x000a04, 0x000a05, 0x000a0b, 0x000a0f, 0x000a11, 0x000a13, 0x000a29, 0x000a2a, 0x000a31, 0x000a32, 0x000a34, 
	0x000a35, 0x000a37, 0x000a38, 0x000a3a, 0x000a3c, 0x000a3d, 0x000a3e, 0x000a43, 0x000a47, 0x000a49, 0x000a4b, 0x000a4e, 0x000a51, 0x000a52, 0x000a59, 0x000a5d, 
	0x000a5e, 0x000a5f, 0x000a66, 0x000a77, 0x00a830, 0x00a83a, 
};
#define mxCharSet_Script_Extensions_Han 76
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Han[mxCharSet_Script_Extensions_Han] = {
	0x002e80, 0x002e9a, 0x002e9b, 0x002ef4, 0x002f00, 0x002fd6, 0x003001, 0x003004, 0x003005, 0x003012, 0x003013, 0x003020, 0x003021, 0x00302e, 0x003030, 0x003031, 
	0x003037, 0x003040, 0x0030fb, 0x0030fc, 0x003190, 0x0031a0, 0x0031c0, 0x0031e4, 0x003220, 0x003248, 0x003280, 0x0032b1, 0x0032c0, 0x0032cc, 0x0032ff, 0x003300, 
	0x003358, 0x003371, 0x00337b, 0x003380, 0x0033e0, 0x0033ff, 0x003400, 0x004dc0, 0x004e00, 0x00a000, 0x00a700, 0x00a708, 0x00f900, 0x00fa6e, 0x00fa70, 0x00fada, 
	0x00fe45, 0x00fe47, 0x00ff61, 0x00ff66, 0x016fe2, 0x016fe4, 0x016ff0, 0x016ff2, 0x01d360, 0x01d372, 0x01f250, 0x01f252, 0x020000, 0x02a6e0, 0x02a700, 0x02b73a, 
	0x02b740, 0x02b81e, 0x02b820, 0x02cea2, 0x02ceb0, 0x02ebe1, 0x02f800, 0x02fa1e, 0x030000, 0x03134b, 0x031350, 0x0323b0, 
};
#define mxCharSet_Script_Extensions_Hangul 42
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hangul[mxCharSet_Script_Extensions_Hangul] = {
	0x001100, 0x001200, 0x003001, 0x003004, 0x003008, 0x003012, 0x003013, 0x003020, 0x00302e, 0x003031, 0x003037, 0x003038, 0x0030fb, 0x0030fc, 0x003131, 0x00318f, 
	0x003200, 0x00321f, 0x003260, 0x00327f, 0x00a960, 0x00a97d, 0x00ac00, 0x00d7a4, 0x00d7b0, 0x00d7c7, 0x00d7cb, 0x00d7fc, 0x00fe45, 0x00fe47, 0x00ff61, 0x00ff66, 
	0x00ffa0, 0x00ffbf, 0x00ffc2, 0x00ffc8, 0x00ffca, 0x00ffd0, 0x00ffd2, 0x00ffd8, 0x00ffda, 0x00ffdd, 
};
#define mxCharSet_Script_Extensions_Hanifi_Rohingya 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hanifi_Rohingya[mxCharSet_Script_Extensions_Hanifi_Rohingya] = {
	0x00060c, 0x00060d, 0x00061b, 0x00061c, 0x00061f, 0x000620, 0x000640, 0x000641, 0x0006d4, 0x0006d5, 0x010d00, 0x010d28, 0x010d30, 0x010d3a, 
};
#define mxCharSet_Script_Extensions_Hanunoo 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hanunoo[mxCharSet_Script_Extensions_Hanunoo] = {
	0x001720, 0x001737, 
};
#define mxCharSet_Script_Extensions_Hatran 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hatran[mxCharSet_Script_Extensions_Hatran] = {
	0x0108e0, 0x0108f3, 0x0108f4, 0x0108f6, 0x0108fb, 0x010900, 
};
#define mxCharSet_Script_Extensions_Hebrew 18
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hebrew[mxCharSet_Script_Extensions_Hebrew] = {
	0x000591, 0x0005c8, 0x0005d0, 0x0005eb, 0x0005ef, 0x0005f5, 0x00fb1d, 0x00fb37, 0x00fb38, 0x00fb3d, 0x00fb3e, 0x00fb3f, 0x00fb40, 0x00fb42, 0x00fb43, 0x00fb45, 
	0x00fb46, 0x00fb50, 
};
#define mxCharSet_Script_Extensions_Hiragana 34
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Hiragana[mxCharSet_Script_Extensions_Hiragana] = {
	0x003001, 0x003004, 0x003008, 0x003012, 0x003013, 0x003020, 0x003030, 0x003036, 0x003037, 0x003038, 0x00303c, 0x00303e, 0x003041, 0x003097, 0x003099, 0x0030a1, 
	0x0030fb, 0x0030fd, 0x00fe45, 0x00fe47, 0x00ff61, 0x00ff66, 0x00ff70, 0x00ff71, 0x00ff9e, 0x00ffa0, 0x01b001, 0x01b120, 0x01b132, 0x01b133, 0x01b150, 0x01b153, 
	0x01f200, 0x01f201, 
};
#define mxCharSet_Script_Extensions_Imperial_Aramaic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Imperial_Aramaic[mxCharSet_Script_Extensions_Imperial_Aramaic] = {
	0x010840, 0x010856, 0x010857, 0x010860, 
};
#define mxCharSet_Script_Extensions_Inherited 40
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Inherited[mxCharSet_Script_Extensions_Inherited] = {
	0x000300, 0x000342, 0x000343, 0x000345, 0x000346, 0x000363, 0x000953, 0x000955, 0x001ab0, 0x001acf, 0x001dc2, 0x001df8, 0x001df9, 0x001dfa, 0x001dfb, 0x001e00, 
	0x00200c, 0x00200e, 0x0020d0, 0x0020f0, 0x00fe00, 0x00fe10, 0x00fe20, 0x00fe2e, 0x0101fd, 0x0101fe, 0x01cf00, 0x01cf2e, 0x01cf30, 0x01cf47, 0x01d167, 0x01d16a, 
	0x01d17b, 0x01d183, 0x01d185, 0x01d18c, 0x01d1aa, 0x01d1ae, 0x0e0100, 0x0e01f0, 
};
#define mxCharSet_Script_Extensions_Inscriptional_Pahlavi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Inscriptional_Pahlavi[mxCharSet_Script_Extensions_Inscriptional_Pahlavi] = {
	0x010b60, 0x010b73, 0x010b78, 0x010b80, 
};
#define mxCharSet_Script_Extensions_Inscriptional_Parthian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Inscriptional_Parthian[mxCharSet_Script_Extensions_Inscriptional_Parthian] = {
	0x010b40, 0x010b56, 0x010b58, 0x010b60, 
};
#define mxCharSet_Script_Extensions_Javanese 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Javanese[mxCharSet_Script_Extensions_Javanese] = {
	0x00a980, 0x00a9ce, 0x00a9cf, 0x00a9da, 0x00a9de, 0x00a9e0, 
};
#define mxCharSet_Script_Extensions_Kaithi 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Kaithi[mxCharSet_Script_Extensions_Kaithi] = {
	0x000966, 0x000970, 0x00a830, 0x00a83a, 0x011080, 0x0110c3, 0x0110cd, 0x0110ce, 
};
#define mxCharSet_Script_Extensions_Kannada 42
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Kannada[mxCharSet_Script_Extensions_Kannada] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000c80, 0x000c8d, 0x000c8e, 0x000c91, 0x000c92, 0x000ca9, 0x000caa, 0x000cb4, 0x000cb5, 0x000cba, 0x000cbc, 0x000cc5, 
	0x000cc6, 0x000cc9, 0x000cca, 0x000cce, 0x000cd5, 0x000cd7, 0x000cdd, 0x000cdf, 0x000ce0, 0x000ce4, 0x000ce6, 0x000cf0, 0x000cf1, 0x000cf4, 0x001cd0, 0x001cd1, 
	0x001cd2, 0x001cd3, 0x001cda, 0x001cdb, 0x001cf2, 0x001cf3, 0x001cf4, 0x001cf5, 0x00a830, 0x00a836, 
};
#define mxCharSet_Script_Extensions_Katakana 40
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Katakana[mxCharSet_Script_Extensions_Katakana] = {
	0x003001, 0x003004, 0x003008, 0x003012, 0x003013, 0x003020, 0x003030, 0x003036, 0x003037, 0x003038, 0x00303c, 0x00303e, 0x003099, 0x00309d, 0x0030a0, 0x003100, 
	0x0031f0, 0x003200, 0x0032d0, 0x0032ff, 0x003300, 0x003358, 0x00fe45, 0x00fe47, 0x00ff61, 0x00ffa0, 0x01aff0, 0x01aff4, 0x01aff5, 0x01affc, 0x01affd, 0x01afff, 
	0x01b000, 0x01b001, 0x01b120, 0x01b123, 0x01b155, 0x01b156, 0x01b164, 0x01b168, 
};
#define mxCharSet_Script_Extensions_Kawi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Kawi[mxCharSet_Script_Extensions_Kawi] = {
	0x011f00, 0x011f11, 0x011f12, 0x011f3b, 0x011f3e, 0x011f5a, 
};
#define mxCharSet_Script_Extensions_Kayah_Li 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Kayah_Li[mxCharSet_Script_Extensions_Kayah_Li] = {
	0x00a900, 0x00a930, 
};
#define mxCharSet_Script_Extensions_Kharoshthi 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Kharoshthi[mxCharSet_Script_Extensions_Kharoshthi] = {
	0x010a00, 0x010a04, 0x010a05, 0x010a07, 0x010a0c, 0x010a14, 0x010a15, 0x010a18, 0x010a19, 0x010a36, 0x010a38, 0x010a3b, 0x010a3f, 0x010a49, 0x010a50, 0x010a59, 
};
#define mxCharSet_Script_Extensions_Khitan_Small_Script 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Khitan_Small_Script[mxCharSet_Script_Extensions_Khitan_Small_Script] = {
	0x016fe4, 0x016fe5, 0x018b00, 0x018cd6, 
};
#define mxCharSet_Script_Extensions_Khmer 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Khmer[mxCharSet_Script_Extensions_Khmer] = {
	0x001780, 0x0017de, 0x0017e0, 0x0017ea, 0x0017f0, 0x0017fa, 0x0019e0, 0x001a00, 
};
#define mxCharSet_Script_Extensions_Khojki 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Khojki[mxCharSet_Script_Extensions_Khojki] = {
	0x000ae6, 0x000af0, 0x00a830, 0x00a83a, 0x011200, 0x011212, 0x011213, 0x011242, 
};
#define mxCharSet_Script_Extensions_Khudawadi 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Khudawadi[mxCharSet_Script_Extensions_Khudawadi] = {
	0x000964, 0x000966, 0x00a830, 0x00a83a, 0x0112b0, 0x0112eb, 0x0112f0, 0x0112fa, 
};
#define mxCharSet_Script_Extensions_Lao 22
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Lao[mxCharSet_Script_Extensions_Lao] = {
	0x000e81, 0x000e83, 0x000e84, 0x000e85, 0x000e86, 0x000e8b, 0x000e8c, 0x000ea4, 0x000ea5, 0x000ea6, 0x000ea7, 0x000ebe, 0x000ec0, 0x000ec5, 0x000ec6, 0x000ec7, 
	0x000ec8, 0x000ecf, 0x000ed0, 0x000eda, 0x000edc, 0x000ee0, 
};
#define mxCharSet_Script_Extensions_Latin 94
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Latin[mxCharSet_Script_Extensions_Latin] = {
	0x000041, 0x00005b, 0x000061, 0x00007b, 0x0000aa, 0x0000ab, 0x0000ba, 0x0000bb, 0x0000c0, 0x0000d7, 0x0000d8, 0x0000f7, 0x0000f8, 0x0002b9, 0x0002e0, 0x0002e5, 
	0x000363, 0x000370, 0x000485, 0x000487, 0x000951, 0x000953, 0x0010fb, 0x0010fc, 0x001d00, 0x001d26, 0x001d2c, 0x001d5d, 0x001d62, 0x001d66, 0x001d6b, 0x001d78, 
	0x001d79, 0x001dbf, 0x001e00, 0x001f00, 0x00202f, 0x002030, 0x002071, 0x002072, 0x00207f, 0x002080, 0x002090, 0x00209d, 0x0020f0, 0x0020f1, 0x00212a, 0x00212c, 
	0x002132, 0x002133, 0x00214e, 0x00214f, 0x002160, 0x002189, 0x002c60, 0x002c80, 0x00a700, 0x00a708, 0x00a722, 0x00a788, 0x00a78b, 0x00a7cb, 0x00a7d0, 0x00a7d2, 
	0x00a7d3, 0x00a7d4, 0x00a7d5, 0x00a7da, 0x00a7f2, 0x00a800, 0x00a92e, 0x00a92f, 0x00ab30, 0x00ab5b, 0x00ab5c, 0x00ab65, 0x00ab66, 0x00ab6a, 0x00fb00, 0x00fb07, 
	0x00ff21, 0x00ff3b, 0x00ff41, 0x00ff5b, 0x010780, 0x010786, 0x010787, 0x0107b1, 0x0107b2, 0x0107bb, 0x01df00, 0x01df1f, 0x01df25, 0x01df2b, 
};
#define mxCharSet_Script_Extensions_Lepcha 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Lepcha[mxCharSet_Script_Extensions_Lepcha] = {
	0x001c00, 0x001c38, 0x001c3b, 0x001c4a, 0x001c4d, 0x001c50, 
};
#define mxCharSet_Script_Extensions_Limbu 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Limbu[mxCharSet_Script_Extensions_Limbu] = {
	0x000965, 0x000966, 0x001900, 0x00191f, 0x001920, 0x00192c, 0x001930, 0x00193c, 0x001940, 0x001941, 0x001944, 0x001950, 
};
#define mxCharSet_Script_Extensions_Linear_A 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Linear_A[mxCharSet_Script_Extensions_Linear_A] = {
	0x010107, 0x010134, 0x010600, 0x010737, 0x010740, 0x010756, 0x010760, 0x010768, 
};
#define mxCharSet_Script_Extensions_Linear_B 20
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Linear_B[mxCharSet_Script_Extensions_Linear_B] = {
	0x010000, 0x01000c, 0x01000d, 0x010027, 0x010028, 0x01003b, 0x01003c, 0x01003e, 0x01003f, 0x01004e, 0x010050, 0x01005e, 0x010080, 0x0100fb, 0x010100, 0x010103, 
	0x010107, 0x010134, 0x010137, 0x010140, 
};
#define mxCharSet_Script_Extensions_Lisu 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Lisu[mxCharSet_Script_Extensions_Lisu] = {
	0x00a4d0, 0x00a500, 0x011fb0, 0x011fb1, 
};
#define mxCharSet_Script_Extensions_Lycian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Lycian[mxCharSet_Script_Extensions_Lycian] = {
	0x010280, 0x01029d, 
};
#define mxCharSet_Script_Extensions_Lydian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Lydian[mxCharSet_Script_Extensions_Lydian] = {
	0x010920, 0x01093a, 0x01093f, 0x010940, 
};
#define mxCharSet_Script_Extensions_Mahajani 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Mahajani[mxCharSet_Script_Extensions_Mahajani] = {
	0x000964, 0x000970, 0x00a830, 0x00a83a, 0x011150, 0x011177, 
};
#define mxCharSet_Script_Extensions_Makasar 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Makasar[mxCharSet_Script_Extensions_Makasar] = {
	0x011ee0, 0x011ef9, 
};
#define mxCharSet_Script_Extensions_Malayalam 22
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Malayalam[mxCharSet_Script_Extensions_Malayalam] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000d00, 0x000d0d, 0x000d0e, 0x000d11, 0x000d12, 0x000d45, 0x000d46, 0x000d49, 0x000d4a, 0x000d50, 0x000d54, 0x000d64, 
	0x000d66, 0x000d80, 0x001cda, 0x001cdb, 0x00a830, 0x00a833, 
};
#define mxCharSet_Script_Extensions_Mandaic 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Mandaic[mxCharSet_Script_Extensions_Mandaic] = {
	0x000640, 0x000641, 0x000840, 0x00085c, 0x00085e, 0x00085f, 
};
#define mxCharSet_Script_Extensions_Manichaean 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Manichaean[mxCharSet_Script_Extensions_Manichaean] = {
	0x000640, 0x000641, 0x010ac0, 0x010ae7, 0x010aeb, 0x010af7, 
};
#define mxCharSet_Script_Extensions_Marchen 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Marchen[mxCharSet_Script_Extensions_Marchen] = {
	0x011c70, 0x011c90, 0x011c92, 0x011ca8, 0x011ca9, 0x011cb7, 
};
#define mxCharSet_Script_Extensions_Masaram_Gondi 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Masaram_Gondi[mxCharSet_Script_Extensions_Masaram_Gondi] = {
	0x000964, 0x000966, 0x011d00, 0x011d07, 0x011d08, 0x011d0a, 0x011d0b, 0x011d37, 0x011d3a, 0x011d3b, 0x011d3c, 0x011d3e, 0x011d3f, 0x011d48, 0x011d50, 0x011d5a, 
};
#define mxCharSet_Script_Extensions_Medefaidrin 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Medefaidrin[mxCharSet_Script_Extensions_Medefaidrin] = {
	0x016e40, 0x016e9b, 
};
#define mxCharSet_Script_Extensions_Meetei_Mayek 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Meetei_Mayek[mxCharSet_Script_Extensions_Meetei_Mayek] = {
	0x00aae0, 0x00aaf7, 0x00abc0, 0x00abee, 0x00abf0, 0x00abfa, 
};
#define mxCharSet_Script_Extensions_Mende_Kikakui 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Mende_Kikakui[mxCharSet_Script_Extensions_Mende_Kikakui] = {
	0x01e800, 0x01e8c5, 0x01e8c7, 0x01e8d7, 
};
#define mxCharSet_Script_Extensions_Meroitic_Cursive 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Meroitic_Cursive[mxCharSet_Script_Extensions_Meroitic_Cursive] = {
	0x0109a0, 0x0109b8, 0x0109bc, 0x0109d0, 0x0109d2, 0x010a00, 
};
#define mxCharSet_Script_Extensions_Meroitic_Hieroglyphs 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Meroitic_Hieroglyphs[mxCharSet_Script_Extensions_Meroitic_Hieroglyphs] = {
	0x010980, 0x0109a0, 
};
#define mxCharSet_Script_Extensions_Miao 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Miao[mxCharSet_Script_Extensions_Miao] = {
	0x016f00, 0x016f4b, 0x016f4f, 0x016f88, 0x016f8f, 0x016fa0, 
};
#define mxCharSet_Script_Extensions_Modi 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Modi[mxCharSet_Script_Extensions_Modi] = {
	0x00a830, 0x00a83a, 0x011600, 0x011645, 0x011650, 0x01165a, 
};
#define mxCharSet_Script_Extensions_Mongolian 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Mongolian[mxCharSet_Script_Extensions_Mongolian] = {
	0x001800, 0x00181a, 0x001820, 0x001879, 0x001880, 0x0018ab, 0x00202f, 0x002030, 0x011660, 0x01166d, 
};
#define mxCharSet_Script_Extensions_Mro 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Mro[mxCharSet_Script_Extensions_Mro] = {
	0x016a40, 0x016a5f, 0x016a60, 0x016a6a, 0x016a6e, 0x016a70, 
};
#define mxCharSet_Script_Extensions_Multani 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Multani[mxCharSet_Script_Extensions_Multani] = {
	0x000a66, 0x000a70, 0x011280, 0x011287, 0x011288, 0x011289, 0x01128a, 0x01128e, 0x01128f, 0x01129e, 0x01129f, 0x0112aa, 
};
#define mxCharSet_Script_Extensions_Myanmar 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Myanmar[mxCharSet_Script_Extensions_Myanmar] = {
	0x001000, 0x0010a0, 0x00a92e, 0x00a92f, 0x00a9e0, 0x00a9ff, 0x00aa60, 0x00aa80, 
};
#define mxCharSet_Script_Extensions_Nabataean 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nabataean[mxCharSet_Script_Extensions_Nabataean] = {
	0x010880, 0x01089f, 0x0108a7, 0x0108b0, 
};
#define mxCharSet_Script_Extensions_Nag_Mundari 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nag_Mundari[mxCharSet_Script_Extensions_Nag_Mundari] = {
	0x01e4d0, 0x01e4fa, 
};
#define mxCharSet_Script_Extensions_Nandinagari 18
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nandinagari[mxCharSet_Script_Extensions_Nandinagari] = {
	0x000964, 0x000966, 0x000ce6, 0x000cf0, 0x001ce9, 0x001cea, 0x001cf2, 0x001cf3, 0x001cfa, 0x001cfb, 0x00a830, 0x00a836, 0x0119a0, 0x0119a8, 0x0119aa, 0x0119d8, 
	0x0119da, 0x0119e5, 
};
#define mxCharSet_Script_Extensions_New_Tai_Lue 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_New_Tai_Lue[mxCharSet_Script_Extensions_New_Tai_Lue] = {
	0x001980, 0x0019ac, 0x0019b0, 0x0019ca, 0x0019d0, 0x0019db, 0x0019de, 0x0019e0, 
};
#define mxCharSet_Script_Extensions_Newa 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Newa[mxCharSet_Script_Extensions_Newa] = {
	0x011400, 0x01145c, 0x01145d, 0x011462, 
};
#define mxCharSet_Script_Extensions_Nko 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nko[mxCharSet_Script_Extensions_Nko] = {
	0x00060c, 0x00060d, 0x00061b, 0x00061c, 0x00061f, 0x000620, 0x0007c0, 0x0007fb, 0x0007fd, 0x000800, 0x00fd3e, 0x00fd40, 
};
#define mxCharSet_Script_Extensions_Nushu 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nushu[mxCharSet_Script_Extensions_Nushu] = {
	0x016fe1, 0x016fe2, 0x01b170, 0x01b2fc, 
};
#define mxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong[mxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong] = {
	0x01e100, 0x01e12d, 0x01e130, 0x01e13e, 0x01e140, 0x01e14a, 0x01e14e, 0x01e150, 
};
#define mxCharSet_Script_Extensions_Ogham 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Ogham[mxCharSet_Script_Extensions_Ogham] = {
	0x001680, 0x00169d, 
};
#define mxCharSet_Script_Extensions_Ol_Chiki 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Ol_Chiki[mxCharSet_Script_Extensions_Ol_Chiki] = {
	0x001c50, 0x001c80, 
};
#define mxCharSet_Script_Extensions_Old_Hungarian 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Hungarian[mxCharSet_Script_Extensions_Old_Hungarian] = {
	0x010c80, 0x010cb3, 0x010cc0, 0x010cf3, 0x010cfa, 0x010d00, 
};
#define mxCharSet_Script_Extensions_Old_Italic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Italic[mxCharSet_Script_Extensions_Old_Italic] = {
	0x010300, 0x010324, 0x01032d, 0x010330, 
};
#define mxCharSet_Script_Extensions_Old_North_Arabian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_North_Arabian[mxCharSet_Script_Extensions_Old_North_Arabian] = {
	0x010a80, 0x010aa0, 
};
#define mxCharSet_Script_Extensions_Old_Permic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Permic[mxCharSet_Script_Extensions_Old_Permic] = {
	0x000483, 0x000484, 0x010350, 0x01037b, 
};
#define mxCharSet_Script_Extensions_Old_Persian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Persian[mxCharSet_Script_Extensions_Old_Persian] = {
	0x0103a0, 0x0103c4, 0x0103c8, 0x0103d6, 
};
#define mxCharSet_Script_Extensions_Old_Sogdian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Sogdian[mxCharSet_Script_Extensions_Old_Sogdian] = {
	0x010f00, 0x010f28, 
};
#define mxCharSet_Script_Extensions_Old_South_Arabian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_South_Arabian[mxCharSet_Script_Extensions_Old_South_Arabian] = {
	0x010a60, 0x010a80, 
};
#define mxCharSet_Script_Extensions_Old_Turkic 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Turkic[mxCharSet_Script_Extensions_Old_Turkic] = {
	0x010c00, 0x010c49, 
};
#define mxCharSet_Script_Extensions_Old_Uyghur 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Old_Uyghur[mxCharSet_Script_Extensions_Old_Uyghur] = {
	0x000640, 0x000641, 0x010af2, 0x010af3, 0x010f70, 0x010f8a, 
};
#define mxCharSet_Script_Extensions_Oriya 36
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Oriya[mxCharSet_Script_Extensions_Oriya] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000b01, 0x000b04, 0x000b05, 0x000b0d, 0x000b0f, 0x000b11, 0x000b13, 0x000b29, 0x000b2a, 0x000b31, 0x000b32, 0x000b34, 
	0x000b35, 0x000b3a, 0x000b3c, 0x000b45, 0x000b47, 0x000b49, 0x000b4b, 0x000b4e, 0x000b55, 0x000b58, 0x000b5c, 0x000b5e, 0x000b5f, 0x000b64, 0x000b66, 0x000b78, 
	0x001cda, 0x001cdb, 0x001cf2, 0x001cf3, 
};
#define mxCharSet_Script_Extensions_Osage 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Osage[mxCharSet_Script_Extensions_Osage] = {
	0x0104b0, 0x0104d4, 0x0104d8, 0x0104fc, 
};
#define mxCharSet_Script_Extensions_Osmanya 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Osmanya[mxCharSet_Script_Extensions_Osmanya] = {
	0x010480, 0x01049e, 0x0104a0, 0x0104aa, 
};
#define mxCharSet_Script_Extensions_Pahawh_Hmong 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Pahawh_Hmong[mxCharSet_Script_Extensions_Pahawh_Hmong] = {
	0x016b00, 0x016b46, 0x016b50, 0x016b5a, 0x016b5b, 0x016b62, 0x016b63, 0x016b78, 0x016b7d, 0x016b90, 
};
#define mxCharSet_Script_Extensions_Palmyrene 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Palmyrene[mxCharSet_Script_Extensions_Palmyrene] = {
	0x010860, 0x010880, 
};
#define mxCharSet_Script_Extensions_Pau_Cin_Hau 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Pau_Cin_Hau[mxCharSet_Script_Extensions_Pau_Cin_Hau] = {
	0x011ac0, 0x011af9, 
};
#define mxCharSet_Script_Extensions_Phags_Pa 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Phags_Pa[mxCharSet_Script_Extensions_Phags_Pa] = {
	0x001802, 0x001804, 0x001805, 0x001806, 0x00a840, 0x00a878, 
};
#define mxCharSet_Script_Extensions_Phoenician 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Phoenician[mxCharSet_Script_Extensions_Phoenician] = {
	0x010900, 0x01091c, 0x01091f, 0x010920, 
};
#define mxCharSet_Script_Extensions_Psalter_Pahlavi 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Psalter_Pahlavi[mxCharSet_Script_Extensions_Psalter_Pahlavi] = {
	0x000640, 0x000641, 0x010b80, 0x010b92, 0x010b99, 0x010b9d, 0x010ba9, 0x010bb0, 
};
#define mxCharSet_Script_Extensions_Rejang 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Rejang[mxCharSet_Script_Extensions_Rejang] = {
	0x00a930, 0x00a954, 0x00a95f, 0x00a960, 
};
#define mxCharSet_Script_Extensions_Runic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Runic[mxCharSet_Script_Extensions_Runic] = {
	0x0016a0, 0x0016eb, 0x0016ee, 0x0016f9, 
};
#define mxCharSet_Script_Extensions_Samaritan 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Samaritan[mxCharSet_Script_Extensions_Samaritan] = {
	0x000800, 0x00082e, 0x000830, 0x00083f, 
};
#define mxCharSet_Script_Extensions_Saurashtra 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Saurashtra[mxCharSet_Script_Extensions_Saurashtra] = {
	0x00a880, 0x00a8c6, 0x00a8ce, 0x00a8da, 
};
#define mxCharSet_Script_Extensions_Sharada 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Sharada[mxCharSet_Script_Extensions_Sharada] = {
	0x000951, 0x000952, 0x001cd7, 0x001cd8, 0x001cd9, 0x001cda, 0x001cdc, 0x001cde, 0x001ce0, 0x001ce1, 0x011180, 0x0111e0, 
};
#define mxCharSet_Script_Extensions_Shavian 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Shavian[mxCharSet_Script_Extensions_Shavian] = {
	0x010450, 0x010480, 
};
#define mxCharSet_Script_Extensions_Siddham 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Siddham[mxCharSet_Script_Extensions_Siddham] = {
	0x011580, 0x0115b6, 0x0115b8, 0x0115de, 
};
#define mxCharSet_Script_Extensions_SignWriting 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_SignWriting[mxCharSet_Script_Extensions_SignWriting] = {
	0x01d800, 0x01da8c, 0x01da9b, 0x01daa0, 0x01daa1, 0x01dab0, 
};
#define mxCharSet_Script_Extensions_Sinhala 28
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Sinhala[mxCharSet_Script_Extensions_Sinhala] = {
	0x000964, 0x000966, 0x000d81, 0x000d84, 0x000d85, 0x000d97, 0x000d9a, 0x000db2, 0x000db3, 0x000dbc, 0x000dbd, 0x000dbe, 0x000dc0, 0x000dc7, 0x000dca, 0x000dcb, 
	0x000dcf, 0x000dd5, 0x000dd6, 0x000dd7, 0x000dd8, 0x000de0, 0x000de6, 0x000df0, 0x000df2, 0x000df5, 0x0111e1, 0x0111f5, 
};
#define mxCharSet_Script_Extensions_Sogdian 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Sogdian[mxCharSet_Script_Extensions_Sogdian] = {
	0x000640, 0x000641, 0x010f30, 0x010f5a, 
};
#define mxCharSet_Script_Extensions_Sora_Sompeng 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Sora_Sompeng[mxCharSet_Script_Extensions_Sora_Sompeng] = {
	0x0110d0, 0x0110e9, 0x0110f0, 0x0110fa, 
};
#define mxCharSet_Script_Extensions_Soyombo 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Soyombo[mxCharSet_Script_Extensions_Soyombo] = {
	0x011a50, 0x011aa3, 
};
#define mxCharSet_Script_Extensions_Sundanese 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Sundanese[mxCharSet_Script_Extensions_Sundanese] = {
	0x001b80, 0x001bc0, 0x001cc0, 0x001cc8, 
};
#define mxCharSet_Script_Extensions_Syloti_Nagri 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Syloti_Nagri[mxCharSet_Script_Extensions_Syloti_Nagri] = {
	0x000964, 0x000966, 0x0009e6, 0x0009f0, 0x00a800, 0x00a82d, 
};
#define mxCharSet_Script_Extensions_Syriac 24
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Syriac[mxCharSet_Script_Extensions_Syriac] = {
	0x00060c, 0x00060d, 0x00061b, 0x00061d, 0x00061f, 0x000620, 0x000640, 0x000641, 0x00064b, 0x000656, 0x000670, 0x000671, 0x000700, 0x00070e, 0x00070f, 0x00074b, 
	0x00074d, 0x000750, 0x000860, 0x00086b, 0x001df8, 0x001df9, 0x001dfa, 0x001dfb, 
};
#define mxCharSet_Script_Extensions_Tagalog 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tagalog[mxCharSet_Script_Extensions_Tagalog] = {
	0x001700, 0x001716, 0x00171f, 0x001720, 0x001735, 0x001737, 
};
#define mxCharSet_Script_Extensions_Tagbanwa 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tagbanwa[mxCharSet_Script_Extensions_Tagbanwa] = {
	0x001735, 0x001737, 0x001760, 0x00176d, 0x00176e, 0x001771, 0x001772, 0x001774, 
};
#define mxCharSet_Script_Extensions_Tai_Le 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tai_Le[mxCharSet_Script_Extensions_Tai_Le] = {
	0x001040, 0x00104a, 0x001950, 0x00196e, 0x001970, 0x001975, 
};
#define mxCharSet_Script_Extensions_Tai_Tham 10
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tai_Tham[mxCharSet_Script_Extensions_Tai_Tham] = {
	0x001a20, 0x001a5f, 0x001a60, 0x001a7d, 0x001a7f, 0x001a8a, 0x001a90, 0x001a9a, 0x001aa0, 0x001aae, 
};
#define mxCharSet_Script_Extensions_Tai_Viet 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tai_Viet[mxCharSet_Script_Extensions_Tai_Viet] = {
	0x00aa80, 0x00aac3, 0x00aadb, 0x00aae0, 
};
#define mxCharSet_Script_Extensions_Takri 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Takri[mxCharSet_Script_Extensions_Takri] = {
	0x000964, 0x000966, 0x00a830, 0x00a83a, 0x011680, 0x0116ba, 0x0116c0, 0x0116ca, 
};
#define mxCharSet_Script_Extensions_Tamil 50
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tamil[mxCharSet_Script_Extensions_Tamil] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000b82, 0x000b84, 0x000b85, 0x000b8b, 0x000b8e, 0x000b91, 0x000b92, 0x000b96, 0x000b99, 0x000b9b, 0x000b9c, 0x000b9d, 
	0x000b9e, 0x000ba0, 0x000ba3, 0x000ba5, 0x000ba8, 0x000bab, 0x000bae, 0x000bba, 0x000bbe, 0x000bc3, 0x000bc6, 0x000bc9, 0x000bca, 0x000bce, 0x000bd0, 0x000bd1, 
	0x000bd7, 0x000bd8, 0x000be6, 0x000bfb, 0x001cda, 0x001cdb, 0x00a8f3, 0x00a8f4, 0x011301, 0x011302, 0x011303, 0x011304, 0x01133b, 0x01133d, 0x011fc0, 0x011ff2, 
	0x011fff, 0x012000, 
};
#define mxCharSet_Script_Extensions_Tangsa 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tangsa[mxCharSet_Script_Extensions_Tangsa] = {
	0x016a70, 0x016abf, 0x016ac0, 0x016aca, 
};
#define mxCharSet_Script_Extensions_Tangut 8
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tangut[mxCharSet_Script_Extensions_Tangut] = {
	0x016fe0, 0x016fe1, 0x017000, 0x0187f8, 0x018800, 0x018b00, 0x018d00, 0x018d09, 
};
#define mxCharSet_Script_Extensions_Telugu 34
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Telugu[mxCharSet_Script_Extensions_Telugu] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x000c00, 0x000c0d, 0x000c0e, 0x000c11, 0x000c12, 0x000c29, 0x000c2a, 0x000c3a, 0x000c3c, 0x000c45, 0x000c46, 0x000c49, 
	0x000c4a, 0x000c4e, 0x000c55, 0x000c57, 0x000c58, 0x000c5b, 0x000c5d, 0x000c5e, 0x000c60, 0x000c64, 0x000c66, 0x000c70, 0x000c77, 0x000c80, 0x001cda, 0x001cdb, 
	0x001cf2, 0x001cf3, 
};
#define mxCharSet_Script_Extensions_Thaana 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Thaana[mxCharSet_Script_Extensions_Thaana] = {
	0x00060c, 0x00060d, 0x00061b, 0x00061d, 0x00061f, 0x000620, 0x000660, 0x00066a, 0x000780, 0x0007b2, 0x00fdf2, 0x00fdf3, 0x00fdfd, 0x00fdfe, 
};
#define mxCharSet_Script_Extensions_Thai 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Thai[mxCharSet_Script_Extensions_Thai] = {
	0x000e01, 0x000e3b, 0x000e40, 0x000e5c, 
};
#define mxCharSet_Script_Extensions_Tibetan 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tibetan[mxCharSet_Script_Extensions_Tibetan] = {
	0x000f00, 0x000f48, 0x000f49, 0x000f6d, 0x000f71, 0x000f98, 0x000f99, 0x000fbd, 0x000fbe, 0x000fcd, 0x000fce, 0x000fd5, 0x000fd9, 0x000fdb, 
};
#define mxCharSet_Script_Extensions_Tifinagh 6
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tifinagh[mxCharSet_Script_Extensions_Tifinagh] = {
	0x002d30, 0x002d68, 0x002d6f, 0x002d71, 0x002d7f, 0x002d80, 
};
#define mxCharSet_Script_Extensions_Tirhuta 12
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Tirhuta[mxCharSet_Script_Extensions_Tirhuta] = {
	0x000951, 0x000953, 0x000964, 0x000966, 0x001cf2, 0x001cf3, 0x00a830, 0x00a83a, 0x011480, 0x0114c8, 0x0114d0, 0x0114da, 
};
#define mxCharSet_Script_Extensions_Toto 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Toto[mxCharSet_Script_Extensions_Toto] = {
	0x01e290, 0x01e2af, 
};
#define mxCharSet_Script_Extensions_Ugaritic 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Ugaritic[mxCharSet_Script_Extensions_Ugaritic] = {
	0x010380, 0x01039e, 0x01039f, 0x0103a0, 
};
#define mxCharSet_Script_Extensions_Vai 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Vai[mxCharSet_Script_Extensions_Vai] = {
	0x00a500, 0x00a62c, 
};
#define mxCharSet_Script_Extensions_Vithkuqi 16
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Vithkuqi[mxCharSet_Script_Extensions_Vithkuqi] = {
	0x010570, 0x01057b, 0x01057c, 0x01058b, 0x01058c, 0x010593, 0x010594, 0x010596, 0x010597, 0x0105a2, 0x0105a3, 0x0105b2, 0x0105b3, 0x0105ba, 0x0105bb, 0x0105bd, 
};
#define mxCharSet_Script_Extensions_Wancho 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Wancho[mxCharSet_Script_Extensions_Wancho] = {
	0x01e2c0, 0x01e2fa, 0x01e2ff, 0x01e300, 
};
#define mxCharSet_Script_Extensions_Warang_Citi 4
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Warang_Citi[mxCharSet_Script_Extensions_Warang_Citi] = {
	0x0118a0, 0x0118f3, 0x0118ff, 0x011900, 
};
#define mxCharSet_Script_Extensions_Yezidi 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Yezidi[mxCharSet_Script_Extensions_Yezidi] = {
	0x00060c, 0x00060d, 0x00061b, 0x00061c, 0x00061f, 0x000620, 0x000660, 0x00066a, 0x010e80, 0x010eaa, 0x010eab, 0x010eae, 0x010eb0, 0x010eb2, 
};
#define mxCharSet_Script_Extensions_Yi 14
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Yi[mxCharSet_Script_Extensions_Yi] = {
	0x003001, 0x003003, 0x003008, 0x003012, 0x003014, 0x00301c, 0x0030fb, 0x0030fc, 0x00a000, 0x00a48d, 0x00a490, 0x00a4c7, 0x00ff61, 0x00ff66, 
};
#define mxCharSet_Script_Extensions_Zanabazar_Square 2
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Script_Extensions_Zanabazar_Square[mxCharSet_Script_Extensions_Zanabazar_Square] = {
	0x011a00, 0x011a48, 
};
#define mxCharSet_Binary_Property 98
static const txCharSetUnicodeProperty ICACHE_RODATA_ATTR gxCharSet_Binary_Property[mxCharSet_Binary_Property] = {
	{ "AHex", mxCharSet_Binary_Property_ASCII_Hex_Digit, gxCharSet_Binary_Property_ASCII_Hex_Digit },
	{ "ASCII", mxCharSet_Binary_Property_ASCII, gxCharSet_Binary_Property_ASCII },
	{ "ASCII_Hex_Digit", mxCharSet_Binary_Property_ASCII_Hex_Digit, gxCharSet_Binary_Property_ASCII_Hex_Digit },
	{ "Alpha", mxCharSet_Binary_Property_Alphabetic, gxCharSet_Binary_Property_Alphabetic },
	{ "Alphabetic", mxCharSet_Binary_Property_Alphabetic, gxCharSet_Binary_Property_Alphabetic },
	{ "Any", mxCharSet_Binary_Property_Any, gxCharSet_Binary_Property_Any },
	{ "Assigned", mxCharSet_Binary_Property_Assigned, gxCharSet_Binary_Property_Assigned },
	{ "Bidi_C", mxCharSet_Binary_Property_Bidi_Control, gxCharSet_Binary_Property_Bidi_Control },
	{ "Bidi_Control", mxCharSet_Binary_Property_Bidi_Control, gxCharSet_Binary_Property_Bidi_Control },
	{ "Bidi_M", mxCharSet_Binary_Property_Bidi_Mirrored, gxCharSet_Binary_Property_Bidi_Mirrored },
	{ "Bidi_Mirrored", mxCharSet_Binary_Property_Bidi_Mirrored, gxCharSet_Binary_Property_Bidi_Mirrored },
	{ "CI", mxCharSet_Binary_Property_Case_Ignorable, gxCharSet_Binary_Property_Case_Ignorable },
	{ "CWCF", mxCharSet_Binary_Property_Changes_When_Casefolded, gxCharSet_Binary_Property_Changes_When_Casefolded },
	{ "CWCM", mxCharSet_Binary_Property_Changes_When_Casemapped, gxCharSet_Binary_Property_Changes_When_Casemapped },
	{ "CWKCF", mxCharSet_Binary_Property_Changes_When_NFKC_Casefolded, gxCharSet_Binary_Property_Changes_When_NFKC_Casefolded },
	{ "CWL", mxCharSet_Binary_Property_Changes_When_Lowercased, gxCharSet_Binary_Property_Changes_When_Lowercased },
	{ "CWT", mxCharSet_Binary_Property_Changes_When_Titlecased, gxCharSet_Binary_Property_Changes_When_Titlecased },
	{ "CWU", mxCharSet_Binary_Property_Changes_When_Uppercased, gxCharSet_Binary_Property_Changes_When_Uppercased },
	{ "Case_Ignorable", mxCharSet_Binary_Property_Case_Ignorable, gxCharSet_Binary_Property_Case_Ignorable },
	{ "Cased", mxCharSet_Binary_Property_Cased, gxCharSet_Binary_Property_Cased },
	{ "Changes_When_Casefolded", mxCharSet_Binary_Property_Changes_When_Casefolded, gxCharSet_Binary_Property_Changes_When_Casefolded },
	{ "Changes_When_Casemapped", mxCharSet_Binary_Property_Changes_When_Casemapped, gxCharSet_Binary_Property_Changes_When_Casemapped },
	{ "Changes_When_Lowercased", mxCharSet_Binary_Property_Changes_When_Lowercased, gxCharSet_Binary_Property_Changes_When_Lowercased },
	{ "Changes_When_NFKC_Casefolded", mxCharSet_Binary_Property_Changes_When_NFKC_Casefolded, gxCharSet_Binary_Property_Changes_When_NFKC_Casefolded },
	{ "Changes_When_Titlecased", mxCharSet_Binary_Property_Changes_When_Titlecased, gxCharSet_Binary_Property_Changes_When_Titlecased },
	{ "Changes_When_Uppercased", mxCharSet_Binary_Property_Changes_When_Uppercased, gxCharSet_Binary_Property_Changes_When_Uppercased },
	{ "DI", mxCharSet_Binary_Property_Default_Ignorable_Code_Point, gxCharSet_Binary_Property_Default_Ignorable_Code_Point },
	{ "Dash", mxCharSet_Binary_Property_Dash, gxCharSet_Binary_Property_Dash },
	{ "Default_Ignorable_Code_Point", mxCharSet_Binary_Property_Default_Ignorable_Code_Point, gxCharSet_Binary_Property_Default_Ignorable_Code_Point },
	{ "Dep", mxCharSet_Binary_Property_Deprecated, gxCharSet_Binary_Property_Deprecated },
	{ "Deprecated", mxCharSet_Binary_Property_Deprecated, gxCharSet_Binary_Property_Deprecated },
	{ "Dia", mxCharSet_Binary_Property_Diacritic, gxCharSet_Binary_Property_Diacritic },
	{ "Diacritic", mxCharSet_Binary_Property_Diacritic, gxCharSet_Binary_Property_Diacritic },
	{ "EBase", mxCharSet_Binary_Property_Emoji_Modifier_Base, gxCharSet_Binary_Property_Emoji_Modifier_Base },
	{ "EComp", mxCharSet_Binary_Property_Emoji_Component, gxCharSet_Binary_Property_Emoji_Component },
	{ "EMod", mxCharSet_Binary_Property_Emoji_Modifier, gxCharSet_Binary_Property_Emoji_Modifier },
	{ "EPres", mxCharSet_Binary_Property_Emoji_Presentation, gxCharSet_Binary_Property_Emoji_Presentation },
	{ "Emoji", mxCharSet_Binary_Property_Emoji, gxCharSet_Binary_Property_Emoji },
	{ "Emoji_Component", mxCharSet_Binary_Property_Emoji_Component, gxCharSet_Binary_Property_Emoji_Component },
	{ "Emoji_Modifier", mxCharSet_Binary_Property_Emoji_Modifier, gxCharSet_Binary_Property_Emoji_Modifier },
	{ "Emoji_Modifier_Base", mxCharSet_Binary_Property_Emoji_Modifier_Base, gxCharSet_Binary_Property_Emoji_Modifier_Base },
	{ "Emoji_Presentation", mxCharSet_Binary_Property_Emoji_Presentation, gxCharSet_Binary_Property_Emoji_Presentation },
	{ "Ext", mxCharSet_Binary_Property_Extender, gxCharSet_Binary_Property_Extender },
	{ "ExtPict", mxCharSet_Binary_Property_Extended_Pictographic, gxCharSet_Binary_Property_Extended_Pictographic },
	{ "Extended_Pictographic", mxCharSet_Binary_Property_Extended_Pictographic, gxCharSet_Binary_Property_Extended_Pictographic },
	{ "Extender", mxCharSet_Binary_Property_Extender, gxCharSet_Binary_Property_Extender },
	{ "Gr_Base", mxCharSet_Binary_Property_Grapheme_Base, gxCharSet_Binary_Property_Grapheme_Base },
	{ "Gr_Ext", mxCharSet_Binary_Property_Grapheme_Extend, gxCharSet_Binary_Property_Grapheme_Extend },
	{ "Grapheme_Base", mxCharSet_Binary_Property_Grapheme_Base, gxCharSet_Binary_Property_Grapheme_Base },
	{ "Grapheme_Extend", mxCharSet_Binary_Property_Grapheme_Extend, gxCharSet_Binary_Property_Grapheme_Extend },
	{ "Hex", mxCharSet_Binary_Property_Hex_Digit, gxCharSet_Binary_Property_Hex_Digit },
	{ "Hex_Digit", mxCharSet_Binary_Property_Hex_Digit, gxCharSet_Binary_Property_Hex_Digit },
	{ "IDC", mxCharSet_Binary_Property_ID_Continue, gxCharSet_Binary_Property_ID_Continue },
	{ "IDS", mxCharSet_Binary_Property_ID_Start, gxCharSet_Binary_Property_ID_Start },
	{ "IDSB", mxCharSet_Binary_Property_IDS_Binary_Operator, gxCharSet_Binary_Property_IDS_Binary_Operator },
	{ "IDST", mxCharSet_Binary_Property_IDS_Trinary_Operator, gxCharSet_Binary_Property_IDS_Trinary_Operator },
	{ "IDS_Binary_Operator", mxCharSet_Binary_Property_IDS_Binary_Operator, gxCharSet_Binary_Property_IDS_Binary_Operator },
	{ "IDS_Trinary_Operator", mxCharSet_Binary_Property_IDS_Trinary_Operator, gxCharSet_Binary_Property_IDS_Trinary_Operator },
	{ "ID_Continue", mxCharSet_Binary_Property_ID_Continue, gxCharSet_Binary_Property_ID_Continue },
	{ "ID_Start", mxCharSet_Binary_Property_ID_Start, gxCharSet_Binary_Property_ID_Start },
	{ "Ideo", mxCharSet_Binary_Property_Ideographic, gxCharSet_Binary_Property_Ideographic },
	{ "Ideographic", mxCharSet_Binary_Property_Ideographic, gxCharSet_Binary_Property_Ideographic },
	{ "Join_C", mxCharSet_Binary_Property_Join_Control, gxCharSet_Binary_Property_Join_Control },
	{ "Join_Control", mxCharSet_Binary_Property_Join_Control, gxCharSet_Binary_Property_Join_Control },
	{ "LOE", mxCharSet_Binary_Property_Logical_Order_Exception, gxCharSet_Binary_Property_Logical_Order_Exception },
	{ "Logical_Order_Exception", mxCharSet_Binary_Property_Logical_Order_Exception, gxCharSet_Binary_Property_Logical_Order_Exception },
	{ "Lower", mxCharSet_Binary_Property_Lowercase, gxCharSet_Binary_Property_Lowercase },
	{ "Lowercase", mxCharSet_Binary_Property_Lowercase, gxCharSet_Binary_Property_Lowercase },
	{ "Math", mxCharSet_Binary_Property_Math, gxCharSet_Binary_Property_Math },
	{ "NChar", mxCharSet_Binary_Property_Noncharacter_Code_Point, gxCharSet_Binary_Property_Noncharacter_Code_Point },
	{ "Noncharacter_Code_Point", mxCharSet_Binary_Property_Noncharacter_Code_Point, gxCharSet_Binary_Property_Noncharacter_Code_Point },
	{ "Pat_Syn", mxCharSet_Binary_Property_Pattern_Syntax, gxCharSet_Binary_Property_Pattern_Syntax },
	{ "Pat_WS", mxCharSet_Binary_Property_Pattern_White_Space, gxCharSet_Binary_Property_Pattern_White_Space },
	{ "Pattern_Syntax", mxCharSet_Binary_Property_Pattern_Syntax, gxCharSet_Binary_Property_Pattern_Syntax },
	{ "Pattern_White_Space", mxCharSet_Binary_Property_Pattern_White_Space, gxCharSet_Binary_Property_Pattern_White_Space },
	{ "QMark", mxCharSet_Binary_Property_Quotation_Mark, gxCharSet_Binary_Property_Quotation_Mark },
	{ "Quotation_Mark", mxCharSet_Binary_Property_Quotation_Mark, gxCharSet_Binary_Property_Quotation_Mark },
	{ "RI", mxCharSet_Binary_Property_Regional_Indicator, gxCharSet_Binary_Property_Regional_Indicator },
	{ "Radical", mxCharSet_Binary_Property_Radical, gxCharSet_Binary_Property_Radical },
	{ "Regional_Indicator", mxCharSet_Binary_Property_Regional_Indicator, gxCharSet_Binary_Property_Regional_Indicator },
	{ "SD", mxCharSet_Binary_Property_Soft_Dotted, gxCharSet_Binary_Property_Soft_Dotted },
	{ "STerm", mxCharSet_Binary_Property_Sentence_Terminal, gxCharSet_Binary_Property_Sentence_Terminal },
	{ "Sentence_Terminal", mxCharSet_Binary_Property_Sentence_Terminal, gxCharSet_Binary_Property_Sentence_Terminal },
	{ "Soft_Dotted", mxCharSet_Binary_Property_Soft_Dotted, gxCharSet_Binary_Property_Soft_Dotted },
	{ "Term", mxCharSet_Binary_Property_Terminal_Punctuation, gxCharSet_Binary_Property_Terminal_Punctuation },
	{ "Terminal_Punctuation", mxCharSet_Binary_Property_Terminal_Punctuation, gxCharSet_Binary_Property_Terminal_Punctuation },
	{ "UIdeo", mxCharSet_Binary_Property_Unified_Ideograph, gxCharSet_Binary_Property_Unified_Ideograph },
	{ "Unified_Ideograph", mxCharSet_Binary_Property_Unified_Ideograph, gxCharSet_Binary_Property_Unified_Ideograph },
	{ "Upper", mxCharSet_Binary_Property_Uppercase, gxCharSet_Binary_Property_Uppercase },
	{ "Uppercase", mxCharSet_Binary_Property_Uppercase, gxCharSet_Binary_Property_Uppercase },
	{ "VS", mxCharSet_Binary_Property_Variation_Selector, gxCharSet_Binary_Property_Variation_Selector },
	{ "Variation_Selector", mxCharSet_Binary_Property_Variation_Selector, gxCharSet_Binary_Property_Variation_Selector },
	{ "White_Space", mxCharSet_Binary_Property_White_Space, gxCharSet_Binary_Property_White_Space },
	{ "XIDC", mxCharSet_Binary_Property_XID_Continue, gxCharSet_Binary_Property_XID_Continue },
	{ "XIDS", mxCharSet_Binary_Property_XID_Start, gxCharSet_Binary_Property_XID_Start },
	{ "XID_Continue", mxCharSet_Binary_Property_XID_Continue, gxCharSet_Binary_Property_XID_Continue },
	{ "XID_Start", mxCharSet_Binary_Property_XID_Start, gxCharSet_Binary_Property_XID_Start },
	{ "space", mxCharSet_Binary_Property_White_Space, gxCharSet_Binary_Property_White_Space },
};
#define mxCharSet_General_Category 80
static const txCharSetUnicodeProperty ICACHE_RODATA_ATTR gxCharSet_General_Category[mxCharSet_General_Category] = {
	{ "C", mxCharSet_General_Category_Other, gxCharSet_General_Category_Other },
	{ "Cased_Letter", mxCharSet_General_Category_Cased_Letter, gxCharSet_General_Category_Cased_Letter },
	{ "Cc", mxCharSet_General_Category_Control, gxCharSet_General_Category_Control },
	{ "Cf", mxCharSet_General_Category_Format, gxCharSet_General_Category_Format },
	{ "Close_Punctuation", mxCharSet_General_Category_Close_Punctuation, gxCharSet_General_Category_Close_Punctuation },
	{ "Cn", mxCharSet_General_Category_Unassigned, gxCharSet_General_Category_Unassigned },
	{ "Co", mxCharSet_General_Category_Private_Use, gxCharSet_General_Category_Private_Use },
	{ "Combining_Mark", mxCharSet_General_Category_Mark, gxCharSet_General_Category_Mark },
	{ "Connector_Punctuation", mxCharSet_General_Category_Connector_Punctuation, gxCharSet_General_Category_Connector_Punctuation },
	{ "Control", mxCharSet_General_Category_Control, gxCharSet_General_Category_Control },
	{ "Cs", mxCharSet_General_Category_Surrogate, gxCharSet_General_Category_Surrogate },
	{ "Currency_Symbol", mxCharSet_General_Category_Currency_Symbol, gxCharSet_General_Category_Currency_Symbol },
	{ "Dash_Punctuation", mxCharSet_General_Category_Dash_Punctuation, gxCharSet_General_Category_Dash_Punctuation },
	{ "Decimal_Number", mxCharSet_General_Category_Decimal_Number, gxCharSet_General_Category_Decimal_Number },
	{ "Enclosing_Mark", mxCharSet_General_Category_Enclosing_Mark, gxCharSet_General_Category_Enclosing_Mark },
	{ "Final_Punctuation", mxCharSet_General_Category_Final_Punctuation, gxCharSet_General_Category_Final_Punctuation },
	{ "Format", mxCharSet_General_Category_Format, gxCharSet_General_Category_Format },
	{ "Initial_Punctuation", mxCharSet_General_Category_Initial_Punctuation, gxCharSet_General_Category_Initial_Punctuation },
	{ "L", mxCharSet_General_Category_Letter, gxCharSet_General_Category_Letter },
	{ "LC", mxCharSet_General_Category_Cased_Letter, gxCharSet_General_Category_Cased_Letter },
	{ "Letter", mxCharSet_General_Category_Letter, gxCharSet_General_Category_Letter },
	{ "Letter_Number", mxCharSet_General_Category_Letter_Number, gxCharSet_General_Category_Letter_Number },
	{ "Line_Separator", mxCharSet_General_Category_Line_Separator, gxCharSet_General_Category_Line_Separator },
	{ "Ll", mxCharSet_General_Category_Lowercase_Letter, gxCharSet_General_Category_Lowercase_Letter },
	{ "Lm", mxCharSet_General_Category_Modifier_Letter, gxCharSet_General_Category_Modifier_Letter },
	{ "Lo", mxCharSet_General_Category_Other_Letter, gxCharSet_General_Category_Other_Letter },
	{ "Lowercase_Letter", mxCharSet_General_Category_Lowercase_Letter, gxCharSet_General_Category_Lowercase_Letter },
	{ "Lt", mxCharSet_General_Category_Titlecase_Letter, gxCharSet_General_Category_Titlecase_Letter },
	{ "Lu", mxCharSet_General_Category_Uppercase_Letter, gxCharSet_General_Category_Uppercase_Letter },
	{ "M", mxCharSet_General_Category_Mark, gxCharSet_General_Category_Mark },
	{ "Mark", mxCharSet_General_Category_Mark, gxCharSet_General_Category_Mark },
	{ "Math_Symbol", mxCharSet_General_Category_Math_Symbol, gxCharSet_General_Category_Math_Symbol },
	{ "Mc", mxCharSet_General_Category_Spacing_Mark, gxCharSet_General_Category_Spacing_Mark },
	{ "Me", mxCharSet_General_Category_Enclosing_Mark, gxCharSet_General_Category_Enclosing_Mark },
	{ "Mn", mxCharSet_General_Category_Nonspacing_Mark, gxCharSet_General_Category_Nonspacing_Mark },
	{ "Modifier_Letter", mxCharSet_General_Category_Modifier_Letter, gxCharSet_General_Category_Modifier_Letter },
	{ "Modifier_Symbol", mxCharSet_General_Category_Modifier_Symbol, gxCharSet_General_Category_Modifier_Symbol },
	{ "N", mxCharSet_General_Category_Number, gxCharSet_General_Category_Number },
	{ "Nd", mxCharSet_General_Category_Decimal_Number, gxCharSet_General_Category_Decimal_Number },
	{ "Nl", mxCharSet_General_Category_Letter_Number, gxCharSet_General_Category_Letter_Number },
	{ "No", mxCharSet_General_Category_Other_Number, gxCharSet_General_Category_Other_Number },
	{ "Nonspacing_Mark", mxCharSet_General_Category_Nonspacing_Mark, gxCharSet_General_Category_Nonspacing_Mark },
	{ "Number", mxCharSet_General_Category_Number, gxCharSet_General_Category_Number },
	{ "Open_Punctuation", mxCharSet_General_Category_Open_Punctuation, gxCharSet_General_Category_Open_Punctuation },
	{ "Other", mxCharSet_General_Category_Other, gxCharSet_General_Category_Other },
	{ "Other_Letter", mxCharSet_General_Category_Other_Letter, gxCharSet_General_Category_Other_Letter },
	{ "Other_Number", mxCharSet_General_Category_Other_Number, gxCharSet_General_Category_Other_Number },
	{ "Other_Punctuation", mxCharSet_General_Category_Other_Punctuation, gxCharSet_General_Category_Other_Punctuation },
	{ "Other_Symbol", mxCharSet_General_Category_Other_Symbol, gxCharSet_General_Category_Other_Symbol },
	{ "P", mxCharSet_General_Category_Punctuation, gxCharSet_General_Category_Punctuation },
	{ "Paragraph_Separator", mxCharSet_General_Category_Paragraph_Separator, gxCharSet_General_Category_Paragraph_Separator },
	{ "Pc", mxCharSet_General_Category_Connector_Punctuation, gxCharSet_General_Category_Connector_Punctuation },
	{ "Pd", mxCharSet_General_Category_Dash_Punctuation, gxCharSet_General_Category_Dash_Punctuation },
	{ "Pe", mxCharSet_General_Category_Close_Punctuation, gxCharSet_General_Category_Close_Punctuation },
	{ "Pf", mxCharSet_General_Category_Final_Punctuation, gxCharSet_General_Category_Final_Punctuation },
	{ "Pi", mxCharSet_General_Category_Initial_Punctuation, gxCharSet_General_Category_Initial_Punctuation },
	{ "Po", mxCharSet_General_Category_Other_Punctuation, gxCharSet_General_Category_Other_Punctuation },
	{ "Private_Use", mxCharSet_General_Category_Private_Use, gxCharSet_General_Category_Private_Use },
	{ "Ps", mxCharSet_General_Category_Open_Punctuation, gxCharSet_General_Category_Open_Punctuation },
	{ "Punctuation", mxCharSet_General_Category_Punctuation, gxCharSet_General_Category_Punctuation },
	{ "S", mxCharSet_General_Category_Symbol, gxCharSet_General_Category_Symbol },
	{ "Sc", mxCharSet_General_Category_Currency_Symbol, gxCharSet_General_Category_Currency_Symbol },
	{ "Separator", mxCharSet_General_Category_Separator, gxCharSet_General_Category_Separator },
	{ "Sk", mxCharSet_General_Category_Modifier_Symbol, gxCharSet_General_Category_Modifier_Symbol },
	{ "Sm", mxCharSet_General_Category_Math_Symbol, gxCharSet_General_Category_Math_Symbol },
	{ "So", mxCharSet_General_Category_Other_Symbol, gxCharSet_General_Category_Other_Symbol },
	{ "Space_Separator", mxCharSet_General_Category_Space_Separator, gxCharSet_General_Category_Space_Separator },
	{ "Spacing_Mark", mxCharSet_General_Category_Spacing_Mark, gxCharSet_General_Category_Spacing_Mark },
	{ "Surrogate", mxCharSet_General_Category_Surrogate, gxCharSet_General_Category_Surrogate },
	{ "Symbol", mxCharSet_General_Category_Symbol, gxCharSet_General_Category_Symbol },
	{ "Titlecase_Letter", mxCharSet_General_Category_Titlecase_Letter, gxCharSet_General_Category_Titlecase_Letter },
	{ "Unassigned", mxCharSet_General_Category_Unassigned, gxCharSet_General_Category_Unassigned },
	{ "Uppercase_Letter", mxCharSet_General_Category_Uppercase_Letter, gxCharSet_General_Category_Uppercase_Letter },
	{ "Z", mxCharSet_General_Category_Separator, gxCharSet_General_Category_Separator },
	{ "Zl", mxCharSet_General_Category_Line_Separator, gxCharSet_General_Category_Line_Separator },
	{ "Zp", mxCharSet_General_Category_Paragraph_Separator, gxCharSet_General_Category_Paragraph_Separator },
	{ "Zs", mxCharSet_General_Category_Space_Separator, gxCharSet_General_Category_Space_Separator },
	{ "cntrl", mxCharSet_General_Category_Control, gxCharSet_General_Category_Control },
	{ "digit", mxCharSet_General_Category_Decimal_Number, gxCharSet_General_Category_Decimal_Number },
	{ "punct", mxCharSet_General_Category_Punctuation, gxCharSet_General_Category_Punctuation },
};
#define mxCharSet_Script 320
static const txCharSetUnicodeProperty ICACHE_RODATA_ATTR gxCharSet_Script[mxCharSet_Script] = {
	{ "Adlam", mxCharSet_Script_Adlam, gxCharSet_Script_Adlam },
	{ "Adlm", mxCharSet_Script_Adlam, gxCharSet_Script_Adlam },
	{ "Aghb", mxCharSet_Script_Caucasian_Albanian, gxCharSet_Script_Caucasian_Albanian },
	{ "Ahom", mxCharSet_Script_Ahom, gxCharSet_Script_Ahom },
	{ "Anatolian_Hieroglyphs", mxCharSet_Script_Anatolian_Hieroglyphs, gxCharSet_Script_Anatolian_Hieroglyphs },
	{ "Arab", mxCharSet_Script_Arabic, gxCharSet_Script_Arabic },
	{ "Arabic", mxCharSet_Script_Arabic, gxCharSet_Script_Arabic },
	{ "Armenian", mxCharSet_Script_Armenian, gxCharSet_Script_Armenian },
	{ "Armi", mxCharSet_Script_Imperial_Aramaic, gxCharSet_Script_Imperial_Aramaic },
	{ "Armn", mxCharSet_Script_Armenian, gxCharSet_Script_Armenian },
	{ "Avestan", mxCharSet_Script_Avestan, gxCharSet_Script_Avestan },
	{ "Avst", mxCharSet_Script_Avestan, gxCharSet_Script_Avestan },
	{ "Bali", mxCharSet_Script_Balinese, gxCharSet_Script_Balinese },
	{ "Balinese", mxCharSet_Script_Balinese, gxCharSet_Script_Balinese },
	{ "Bamu", mxCharSet_Script_Bamum, gxCharSet_Script_Bamum },
	{ "Bamum", mxCharSet_Script_Bamum, gxCharSet_Script_Bamum },
	{ "Bass", mxCharSet_Script_Bassa_Vah, gxCharSet_Script_Bassa_Vah },
	{ "Bassa_Vah", mxCharSet_Script_Bassa_Vah, gxCharSet_Script_Bassa_Vah },
	{ "Batak", mxCharSet_Script_Batak, gxCharSet_Script_Batak },
	{ "Batk", mxCharSet_Script_Batak, gxCharSet_Script_Batak },
	{ "Beng", mxCharSet_Script_Bengali, gxCharSet_Script_Bengali },
	{ "Bengali", mxCharSet_Script_Bengali, gxCharSet_Script_Bengali },
	{ "Bhaiksuki", mxCharSet_Script_Bhaiksuki, gxCharSet_Script_Bhaiksuki },
	{ "Bhks", mxCharSet_Script_Bhaiksuki, gxCharSet_Script_Bhaiksuki },
	{ "Bopo", mxCharSet_Script_Bopomofo, gxCharSet_Script_Bopomofo },
	{ "Bopomofo", mxCharSet_Script_Bopomofo, gxCharSet_Script_Bopomofo },
	{ "Brah", mxCharSet_Script_Brahmi, gxCharSet_Script_Brahmi },
	{ "Brahmi", mxCharSet_Script_Brahmi, gxCharSet_Script_Brahmi },
	{ "Brai", mxCharSet_Script_Braille, gxCharSet_Script_Braille },
	{ "Braille", mxCharSet_Script_Braille, gxCharSet_Script_Braille },
	{ "Bugi", mxCharSet_Script_Buginese, gxCharSet_Script_Buginese },
	{ "Buginese", mxCharSet_Script_Buginese, gxCharSet_Script_Buginese },
	{ "Buhd", mxCharSet_Script_Buhid, gxCharSet_Script_Buhid },
	{ "Buhid", mxCharSet_Script_Buhid, gxCharSet_Script_Buhid },
	{ "Cakm", mxCharSet_Script_Chakma, gxCharSet_Script_Chakma },
	{ "Canadian_Aboriginal", mxCharSet_Script_Canadian_Aboriginal, gxCharSet_Script_Canadian_Aboriginal },
	{ "Cans", mxCharSet_Script_Canadian_Aboriginal, gxCharSet_Script_Canadian_Aboriginal },
	{ "Cari", mxCharSet_Script_Carian, gxCharSet_Script_Carian },
	{ "Carian", mxCharSet_Script_Carian, gxCharSet_Script_Carian },
	{ "Caucasian_Albanian", mxCharSet_Script_Caucasian_Albanian, gxCharSet_Script_Caucasian_Albanian },
	{ "Chakma", mxCharSet_Script_Chakma, gxCharSet_Script_Chakma },
	{ "Cham", mxCharSet_Script_Cham, gxCharSet_Script_Cham },
	{ "Cher", mxCharSet_Script_Cherokee, gxCharSet_Script_Cherokee },
	{ "Cherokee", mxCharSet_Script_Cherokee, gxCharSet_Script_Cherokee },
	{ "Chorasmian", mxCharSet_Script_Chorasmian, gxCharSet_Script_Chorasmian },
	{ "Chrs", mxCharSet_Script_Chorasmian, gxCharSet_Script_Chorasmian },
	{ "Common", mxCharSet_Script_Common, gxCharSet_Script_Common },
	{ "Copt", mxCharSet_Script_Coptic, gxCharSet_Script_Coptic },
	{ "Coptic", mxCharSet_Script_Coptic, gxCharSet_Script_Coptic },
	{ "Cpmn", mxCharSet_Script_Cypro_Minoan, gxCharSet_Script_Cypro_Minoan },
	{ "Cprt", mxCharSet_Script_Cypriot, gxCharSet_Script_Cypriot },
	{ "Cuneiform", mxCharSet_Script_Cuneiform, gxCharSet_Script_Cuneiform },
	{ "Cypriot", mxCharSet_Script_Cypriot, gxCharSet_Script_Cypriot },
	{ "Cypro_Minoan", mxCharSet_Script_Cypro_Minoan, gxCharSet_Script_Cypro_Minoan },
	{ "Cyrillic", mxCharSet_Script_Cyrillic, gxCharSet_Script_Cyrillic },
	{ "Cyrl", mxCharSet_Script_Cyrillic, gxCharSet_Script_Cyrillic },
	{ "Deseret", mxCharSet_Script_Deseret, gxCharSet_Script_Deseret },
	{ "Deva", mxCharSet_Script_Devanagari, gxCharSet_Script_Devanagari },
	{ "Devanagari", mxCharSet_Script_Devanagari, gxCharSet_Script_Devanagari },
	{ "Diak", mxCharSet_Script_Dives_Akuru, gxCharSet_Script_Dives_Akuru },
	{ "Dives_Akuru", mxCharSet_Script_Dives_Akuru, gxCharSet_Script_Dives_Akuru },
	{ "Dogr", mxCharSet_Script_Dogra, gxCharSet_Script_Dogra },
	{ "Dogra", mxCharSet_Script_Dogra, gxCharSet_Script_Dogra },
	{ "Dsrt", mxCharSet_Script_Deseret, gxCharSet_Script_Deseret },
	{ "Dupl", mxCharSet_Script_Duployan, gxCharSet_Script_Duployan },
	{ "Duployan", mxCharSet_Script_Duployan, gxCharSet_Script_Duployan },
	{ "Egyp", mxCharSet_Script_Egyptian_Hieroglyphs, gxCharSet_Script_Egyptian_Hieroglyphs },
	{ "Egyptian_Hieroglyphs", mxCharSet_Script_Egyptian_Hieroglyphs, gxCharSet_Script_Egyptian_Hieroglyphs },
	{ "Elba", mxCharSet_Script_Elbasan, gxCharSet_Script_Elbasan },
	{ "Elbasan", mxCharSet_Script_Elbasan, gxCharSet_Script_Elbasan },
	{ "Elym", mxCharSet_Script_Elymaic, gxCharSet_Script_Elymaic },
	{ "Elymaic", mxCharSet_Script_Elymaic, gxCharSet_Script_Elymaic },
	{ "Ethi", mxCharSet_Script_Ethiopic, gxCharSet_Script_Ethiopic },
	{ "Ethiopic", mxCharSet_Script_Ethiopic, gxCharSet_Script_Ethiopic },
	{ "Geor", mxCharSet_Script_Georgian, gxCharSet_Script_Georgian },
	{ "Georgian", mxCharSet_Script_Georgian, gxCharSet_Script_Georgian },
	{ "Glag", mxCharSet_Script_Glagolitic, gxCharSet_Script_Glagolitic },
	{ "Glagolitic", mxCharSet_Script_Glagolitic, gxCharSet_Script_Glagolitic },
	{ "Gong", mxCharSet_Script_Gunjala_Gondi, gxCharSet_Script_Gunjala_Gondi },
	{ "Gonm", mxCharSet_Script_Masaram_Gondi, gxCharSet_Script_Masaram_Gondi },
	{ "Goth", mxCharSet_Script_Gothic, gxCharSet_Script_Gothic },
	{ "Gothic", mxCharSet_Script_Gothic, gxCharSet_Script_Gothic },
	{ "Gran", mxCharSet_Script_Grantha, gxCharSet_Script_Grantha },
	{ "Grantha", mxCharSet_Script_Grantha, gxCharSet_Script_Grantha },
	{ "Greek", mxCharSet_Script_Greek, gxCharSet_Script_Greek },
	{ "Grek", mxCharSet_Script_Greek, gxCharSet_Script_Greek },
	{ "Gujarati", mxCharSet_Script_Gujarati, gxCharSet_Script_Gujarati },
	{ "Gujr", mxCharSet_Script_Gujarati, gxCharSet_Script_Gujarati },
	{ "Gunjala_Gondi", mxCharSet_Script_Gunjala_Gondi, gxCharSet_Script_Gunjala_Gondi },
	{ "Gurmukhi", mxCharSet_Script_Gurmukhi, gxCharSet_Script_Gurmukhi },
	{ "Guru", mxCharSet_Script_Gurmukhi, gxCharSet_Script_Gurmukhi },
	{ "Han", mxCharSet_Script_Han, gxCharSet_Script_Han },
	{ "Hang", mxCharSet_Script_Hangul, gxCharSet_Script_Hangul },
	{ "Hangul", mxCharSet_Script_Hangul, gxCharSet_Script_Hangul },
	{ "Hani", mxCharSet_Script_Han, gxCharSet_Script_Han },
	{ "Hanifi_Rohingya", mxCharSet_Script_Hanifi_Rohingya, gxCharSet_Script_Hanifi_Rohingya },
	{ "Hano", mxCharSet_Script_Hanunoo, gxCharSet_Script_Hanunoo },
	{ "Hanunoo", mxCharSet_Script_Hanunoo, gxCharSet_Script_Hanunoo },
	{ "Hatr", mxCharSet_Script_Hatran, gxCharSet_Script_Hatran },
	{ "Hatran", mxCharSet_Script_Hatran, gxCharSet_Script_Hatran },
	{ "Hebr", mxCharSet_Script_Hebrew, gxCharSet_Script_Hebrew },
	{ "Hebrew", mxCharSet_Script_Hebrew, gxCharSet_Script_Hebrew },
	{ "Hira", mxCharSet_Script_Hiragana, gxCharSet_Script_Hiragana },
	{ "Hiragana", mxCharSet_Script_Hiragana, gxCharSet_Script_Hiragana },
	{ "Hluw", mxCharSet_Script_Anatolian_Hieroglyphs, gxCharSet_Script_Anatolian_Hieroglyphs },
	{ "Hmng", mxCharSet_Script_Pahawh_Hmong, gxCharSet_Script_Pahawh_Hmong },
	{ "Hmnp", mxCharSet_Script_Nyiakeng_Puachue_Hmong, gxCharSet_Script_Nyiakeng_Puachue_Hmong },
	{ "Hung", mxCharSet_Script_Old_Hungarian, gxCharSet_Script_Old_Hungarian },
	{ "Imperial_Aramaic", mxCharSet_Script_Imperial_Aramaic, gxCharSet_Script_Imperial_Aramaic },
	{ "Inherited", mxCharSet_Script_Inherited, gxCharSet_Script_Inherited },
	{ "Inscriptional_Pahlavi", mxCharSet_Script_Inscriptional_Pahlavi, gxCharSet_Script_Inscriptional_Pahlavi },
	{ "Inscriptional_Parthian", mxCharSet_Script_Inscriptional_Parthian, gxCharSet_Script_Inscriptional_Parthian },
	{ "Ital", mxCharSet_Script_Old_Italic, gxCharSet_Script_Old_Italic },
	{ "Java", mxCharSet_Script_Javanese, gxCharSet_Script_Javanese },
	{ "Javanese", mxCharSet_Script_Javanese, gxCharSet_Script_Javanese },
	{ "Kaithi", mxCharSet_Script_Kaithi, gxCharSet_Script_Kaithi },
	{ "Kali", mxCharSet_Script_Kayah_Li, gxCharSet_Script_Kayah_Li },
	{ "Kana", mxCharSet_Script_Katakana, gxCharSet_Script_Katakana },
	{ "Kannada", mxCharSet_Script_Kannada, gxCharSet_Script_Kannada },
	{ "Katakana", mxCharSet_Script_Katakana, gxCharSet_Script_Katakana },
	{ "Kawi", mxCharSet_Script_Kawi, gxCharSet_Script_Kawi },
	{ "Kayah_Li", mxCharSet_Script_Kayah_Li, gxCharSet_Script_Kayah_Li },
	{ "Khar", mxCharSet_Script_Kharoshthi, gxCharSet_Script_Kharoshthi },
	{ "Kharoshthi", mxCharSet_Script_Kharoshthi, gxCharSet_Script_Kharoshthi },
	{ "Khitan_Small_Script", mxCharSet_Script_Khitan_Small_Script, gxCharSet_Script_Khitan_Small_Script },
	{ "Khmer", mxCharSet_Script_Khmer, gxCharSet_Script_Khmer },
	{ "Khmr", mxCharSet_Script_Khmer, gxCharSet_Script_Khmer },
	{ "Khoj", mxCharSet_Script_Khojki, gxCharSet_Script_Khojki },
	{ "Khojki", mxCharSet_Script_Khojki, gxCharSet_Script_Khojki },
	{ "Khudawadi", mxCharSet_Script_Khudawadi, gxCharSet_Script_Khudawadi },
	{ "Kits", mxCharSet_Script_Khitan_Small_Script, gxCharSet_Script_Khitan_Small_Script },
	{ "Knda", mxCharSet_Script_Kannada, gxCharSet_Script_Kannada },
	{ "Kthi", mxCharSet_Script_Kaithi, gxCharSet_Script_Kaithi },
	{ "Lana", mxCharSet_Script_Tai_Tham, gxCharSet_Script_Tai_Tham },
	{ "Lao", mxCharSet_Script_Lao, gxCharSet_Script_Lao },
	{ "Laoo", mxCharSet_Script_Lao, gxCharSet_Script_Lao },
	{ "Latin", mxCharSet_Script_Latin, gxCharSet_Script_Latin },
	{ "Latn", mxCharSet_Script_Latin, gxCharSet_Script_Latin },
	{ "Lepc", mxCharSet_Script_Lepcha, gxCharSet_Script_Lepcha },
	{ "Lepcha", mxCharSet_Script_Lepcha, gxCharSet_Script_Lepcha },
	{ "Limb", mxCharSet_Script_Limbu, gxCharSet_Script_Limbu },
	{ "Limbu", mxCharSet_Script_Limbu, gxCharSet_Script_Limbu },
	{ "Lina", mxCharSet_Script_Linear_A, gxCharSet_Script_Linear_A },
	{ "Linb", mxCharSet_Script_Linear_B, gxCharSet_Script_Linear_B },
	{ "Linear_A", mxCharSet_Script_Linear_A, gxCharSet_Script_Linear_A },
	{ "Linear_B", mxCharSet_Script_Linear_B, gxCharSet_Script_Linear_B },
	{ "Lisu", mxCharSet_Script_Lisu, gxCharSet_Script_Lisu },
	{ "Lyci", mxCharSet_Script_Lycian, gxCharSet_Script_Lycian },
	{ "Lycian", mxCharSet_Script_Lycian, gxCharSet_Script_Lycian },
	{ "Lydi", mxCharSet_Script_Lydian, gxCharSet_Script_Lydian },
	{ "Lydian", mxCharSet_Script_Lydian, gxCharSet_Script_Lydian },
	{ "Mahajani", mxCharSet_Script_Mahajani, gxCharSet_Script_Mahajani },
	{ "Mahj", mxCharSet_Script_Mahajani, gxCharSet_Script_Mahajani },
	{ "Maka", mxCharSet_Script_Makasar, gxCharSet_Script_Makasar },
	{ "Makasar", mxCharSet_Script_Makasar, gxCharSet_Script_Makasar },
	{ "Malayalam", mxCharSet_Script_Malayalam, gxCharSet_Script_Malayalam },
	{ "Mand", mxCharSet_Script_Mandaic, gxCharSet_Script_Mandaic },
	{ "Mandaic", mxCharSet_Script_Mandaic, gxCharSet_Script_Mandaic },
	{ "Mani", mxCharSet_Script_Manichaean, gxCharSet_Script_Manichaean },
	{ "Manichaean", mxCharSet_Script_Manichaean, gxCharSet_Script_Manichaean },
	{ "Marc", mxCharSet_Script_Marchen, gxCharSet_Script_Marchen },
	{ "Marchen", mxCharSet_Script_Marchen, gxCharSet_Script_Marchen },
	{ "Masaram_Gondi", mxCharSet_Script_Masaram_Gondi, gxCharSet_Script_Masaram_Gondi },
	{ "Medefaidrin", mxCharSet_Script_Medefaidrin, gxCharSet_Script_Medefaidrin },
	{ "Medf", mxCharSet_Script_Medefaidrin, gxCharSet_Script_Medefaidrin },
	{ "Meetei_Mayek", mxCharSet_Script_Meetei_Mayek, gxCharSet_Script_Meetei_Mayek },
	{ "Mend", mxCharSet_Script_Mende_Kikakui, gxCharSet_Script_Mende_Kikakui },
	{ "Mende_Kikakui", mxCharSet_Script_Mende_Kikakui, gxCharSet_Script_Mende_Kikakui },
	{ "Merc", mxCharSet_Script_Meroitic_Cursive, gxCharSet_Script_Meroitic_Cursive },
	{ "Mero", mxCharSet_Script_Meroitic_Hieroglyphs, gxCharSet_Script_Meroitic_Hieroglyphs },
	{ "Meroitic_Cursive", mxCharSet_Script_Meroitic_Cursive, gxCharSet_Script_Meroitic_Cursive },
	{ "Meroitic_Hieroglyphs", mxCharSet_Script_Meroitic_Hieroglyphs, gxCharSet_Script_Meroitic_Hieroglyphs },
	{ "Miao", mxCharSet_Script_Miao, gxCharSet_Script_Miao },
	{ "Mlym", mxCharSet_Script_Malayalam, gxCharSet_Script_Malayalam },
	{ "Modi", mxCharSet_Script_Modi, gxCharSet_Script_Modi },
	{ "Mong", mxCharSet_Script_Mongolian, gxCharSet_Script_Mongolian },
	{ "Mongolian", mxCharSet_Script_Mongolian, gxCharSet_Script_Mongolian },
	{ "Mro", mxCharSet_Script_Mro, gxCharSet_Script_Mro },
	{ "Mroo", mxCharSet_Script_Mro, gxCharSet_Script_Mro },
	{ "Mtei", mxCharSet_Script_Meetei_Mayek, gxCharSet_Script_Meetei_Mayek },
	{ "Mult", mxCharSet_Script_Multani, gxCharSet_Script_Multani },
	{ "Multani", mxCharSet_Script_Multani, gxCharSet_Script_Multani },
	{ "Myanmar", mxCharSet_Script_Myanmar, gxCharSet_Script_Myanmar },
	{ "Mymr", mxCharSet_Script_Myanmar, gxCharSet_Script_Myanmar },
	{ "Nabataean", mxCharSet_Script_Nabataean, gxCharSet_Script_Nabataean },
	{ "Nag_Mundari", mxCharSet_Script_Nag_Mundari, gxCharSet_Script_Nag_Mundari },
	{ "Nagm", mxCharSet_Script_Nag_Mundari, gxCharSet_Script_Nag_Mundari },
	{ "Nand", mxCharSet_Script_Nandinagari, gxCharSet_Script_Nandinagari },
	{ "Nandinagari", mxCharSet_Script_Nandinagari, gxCharSet_Script_Nandinagari },
	{ "Narb", mxCharSet_Script_Old_North_Arabian, gxCharSet_Script_Old_North_Arabian },
	{ "Nbat", mxCharSet_Script_Nabataean, gxCharSet_Script_Nabataean },
	{ "New_Tai_Lue", mxCharSet_Script_New_Tai_Lue, gxCharSet_Script_New_Tai_Lue },
	{ "Newa", mxCharSet_Script_Newa, gxCharSet_Script_Newa },
	{ "Nko", mxCharSet_Script_Nko, gxCharSet_Script_Nko },
	{ "Nkoo", mxCharSet_Script_Nko, gxCharSet_Script_Nko },
	{ "Nshu", mxCharSet_Script_Nushu, gxCharSet_Script_Nushu },
	{ "Nushu", mxCharSet_Script_Nushu, gxCharSet_Script_Nushu },
	{ "Nyiakeng_Puachue_Hmong", mxCharSet_Script_Nyiakeng_Puachue_Hmong, gxCharSet_Script_Nyiakeng_Puachue_Hmong },
	{ "Ogam", mxCharSet_Script_Ogham, gxCharSet_Script_Ogham },
	{ "Ogham", mxCharSet_Script_Ogham, gxCharSet_Script_Ogham },
	{ "Ol_Chiki", mxCharSet_Script_Ol_Chiki, gxCharSet_Script_Ol_Chiki },
	{ "Olck", mxCharSet_Script_Ol_Chiki, gxCharSet_Script_Ol_Chiki },
	{ "Old_Hungarian", mxCharSet_Script_Old_Hungarian, gxCharSet_Script_Old_Hungarian },
	{ "Old_Italic", mxCharSet_Script_Old_Italic, gxCharSet_Script_Old_Italic },
	{ "Old_North_Arabian", mxCharSet_Script_Old_North_Arabian, gxCharSet_Script_Old_North_Arabian },
	{ "Old_Permic", mxCharSet_Script_Old_Permic, gxCharSet_Script_Old_Permic },
	{ "Old_Persian", mxCharSet_Script_Old_Persian, gxCharSet_Script_Old_Persian },
	{ "Old_Sogdian", mxCharSet_Script_Old_Sogdian, gxCharSet_Script_Old_Sogdian },
	{ "Old_South_Arabian", mxCharSet_Script_Old_South_Arabian, gxCharSet_Script_Old_South_Arabian },
	{ "Old_Turkic", mxCharSet_Script_Old_Turkic, gxCharSet_Script_Old_Turkic },
	{ "Old_Uyghur", mxCharSet_Script_Old_Uyghur, gxCharSet_Script_Old_Uyghur },
	{ "Oriya", mxCharSet_Script_Oriya, gxCharSet_Script_Oriya },
	{ "Orkh", mxCharSet_Script_Old_Turkic, gxCharSet_Script_Old_Turkic },
	{ "Orya", mxCharSet_Script_Oriya, gxCharSet_Script_Oriya },
	{ "Osage", mxCharSet_Script_Osage, gxCharSet_Script_Osage },
	{ "Osge", mxCharSet_Script_Osage, gxCharSet_Script_Osage },
	{ "Osma", mxCharSet_Script_Osmanya, gxCharSet_Script_Osmanya },
	{ "Osmanya", mxCharSet_Script_Osmanya, gxCharSet_Script_Osmanya },
	{ "Ougr", mxCharSet_Script_Old_Uyghur, gxCharSet_Script_Old_Uyghur },
	{ "Pahawh_Hmong", mxCharSet_Script_Pahawh_Hmong, gxCharSet_Script_Pahawh_Hmong },
	{ "Palm", mxCharSet_Script_Palmyrene, gxCharSet_Script_Palmyrene },
	{ "Palmyrene", mxCharSet_Script_Palmyrene, gxCharSet_Script_Palmyrene },
	{ "Pau_Cin_Hau", mxCharSet_Script_Pau_Cin_Hau, gxCharSet_Script_Pau_Cin_Hau },
	{ "Pauc", mxCharSet_Script_Pau_Cin_Hau, gxCharSet_Script_Pau_Cin_Hau },
	{ "Perm", mxCharSet_Script_Old_Permic, gxCharSet_Script_Old_Permic },
	{ "Phag", mxCharSet_Script_Phags_Pa, gxCharSet_Script_Phags_Pa },
	{ "Phags_Pa", mxCharSet_Script_Phags_Pa, gxCharSet_Script_Phags_Pa },
	{ "Phli", mxCharSet_Script_Inscriptional_Pahlavi, gxCharSet_Script_Inscriptional_Pahlavi },
	{ "Phlp", mxCharSet_Script_Psalter_Pahlavi, gxCharSet_Script_Psalter_Pahlavi },
	{ "Phnx", mxCharSet_Script_Phoenician, gxCharSet_Script_Phoenician },
	{ "Phoenician", mxCharSet_Script_Phoenician, gxCharSet_Script_Phoenician },
	{ "Plrd", mxCharSet_Script_Miao, gxCharSet_Script_Miao },
	{ "Prti", mxCharSet_Script_Inscriptional_Parthian, gxCharSet_Script_Inscriptional_Parthian },
	{ "Psalter_Pahlavi", mxCharSet_Script_Psalter_Pahlavi, gxCharSet_Script_Psalter_Pahlavi },
	{ "Qaac", mxCharSet_Script_Coptic, gxCharSet_Script_Coptic },
	{ "Qaai", mxCharSet_Script_Inherited, gxCharSet_Script_Inherited },
	{ "Rejang", mxCharSet_Script_Rejang, gxCharSet_Script_Rejang },
	{ "Rjng", mxCharSet_Script_Rejang, gxCharSet_Script_Rejang },
	{ "Rohg", mxCharSet_Script_Hanifi_Rohingya, gxCharSet_Script_Hanifi_Rohingya },
	{ "Runic", mxCharSet_Script_Runic, gxCharSet_Script_Runic },
	{ "Runr", mxCharSet_Script_Runic, gxCharSet_Script_Runic },
	{ "Samaritan", mxCharSet_Script_Samaritan, gxCharSet_Script_Samaritan },
	{ "Samr", mxCharSet_Script_Samaritan, gxCharSet_Script_Samaritan },
	{ "Sarb", mxCharSet_Script_Old_South_Arabian, gxCharSet_Script_Old_South_Arabian },
	{ "Saur", mxCharSet_Script_Saurashtra, gxCharSet_Script_Saurashtra },
	{ "Saurashtra", mxCharSet_Script_Saurashtra, gxCharSet_Script_Saurashtra },
	{ "Sgnw", mxCharSet_Script_SignWriting, gxCharSet_Script_SignWriting },
	{ "Sharada", mxCharSet_Script_Sharada, gxCharSet_Script_Sharada },
	{ "Shavian", mxCharSet_Script_Shavian, gxCharSet_Script_Shavian },
	{ "Shaw", mxCharSet_Script_Shavian, gxCharSet_Script_Shavian },
	{ "Shrd", mxCharSet_Script_Sharada, gxCharSet_Script_Sharada },
	{ "Sidd", mxCharSet_Script_Siddham, gxCharSet_Script_Siddham },
	{ "Siddham", mxCharSet_Script_Siddham, gxCharSet_Script_Siddham },
	{ "SignWriting", mxCharSet_Script_SignWriting, gxCharSet_Script_SignWriting },
	{ "Sind", mxCharSet_Script_Khudawadi, gxCharSet_Script_Khudawadi },
	{ "Sinh", mxCharSet_Script_Sinhala, gxCharSet_Script_Sinhala },
	{ "Sinhala", mxCharSet_Script_Sinhala, gxCharSet_Script_Sinhala },
	{ "Sogd", mxCharSet_Script_Sogdian, gxCharSet_Script_Sogdian },
	{ "Sogdian", mxCharSet_Script_Sogdian, gxCharSet_Script_Sogdian },
	{ "Sogo", mxCharSet_Script_Old_Sogdian, gxCharSet_Script_Old_Sogdian },
	{ "Sora", mxCharSet_Script_Sora_Sompeng, gxCharSet_Script_Sora_Sompeng },
	{ "Sora_Sompeng", mxCharSet_Script_Sora_Sompeng, gxCharSet_Script_Sora_Sompeng },
	{ "Soyo", mxCharSet_Script_Soyombo, gxCharSet_Script_Soyombo },
	{ "Soyombo", mxCharSet_Script_Soyombo, gxCharSet_Script_Soyombo },
	{ "Sund", mxCharSet_Script_Sundanese, gxCharSet_Script_Sundanese },
	{ "Sundanese", mxCharSet_Script_Sundanese, gxCharSet_Script_Sundanese },
	{ "Sylo", mxCharSet_Script_Syloti_Nagri, gxCharSet_Script_Syloti_Nagri },
	{ "Syloti_Nagri", mxCharSet_Script_Syloti_Nagri, gxCharSet_Script_Syloti_Nagri },
	{ "Syrc", mxCharSet_Script_Syriac, gxCharSet_Script_Syriac },
	{ "Syriac", mxCharSet_Script_Syriac, gxCharSet_Script_Syriac },
	{ "Tagalog", mxCharSet_Script_Tagalog, gxCharSet_Script_Tagalog },
	{ "Tagb", mxCharSet_Script_Tagbanwa, gxCharSet_Script_Tagbanwa },
	{ "Tagbanwa", mxCharSet_Script_Tagbanwa, gxCharSet_Script_Tagbanwa },
	{ "Tai_Le", mxCharSet_Script_Tai_Le, gxCharSet_Script_Tai_Le },
	{ "Tai_Tham", mxCharSet_Script_Tai_Tham, gxCharSet_Script_Tai_Tham },
	{ "Tai_Viet", mxCharSet_Script_Tai_Viet, gxCharSet_Script_Tai_Viet },
	{ "Takr", mxCharSet_Script_Takri, gxCharSet_Script_Takri },
	{ "Takri", mxCharSet_Script_Takri, gxCharSet_Script_Takri },
	{ "Tale", mxCharSet_Script_Tai_Le, gxCharSet_Script_Tai_Le },
	{ "Talu", mxCharSet_Script_New_Tai_Lue, gxCharSet_Script_New_Tai_Lue },
	{ "Tamil", mxCharSet_Script_Tamil, gxCharSet_Script_Tamil },
	{ "Taml", mxCharSet_Script_Tamil, gxCharSet_Script_Tamil },
	{ "Tang", mxCharSet_Script_Tangut, gxCharSet_Script_Tangut },
	{ "Tangsa", mxCharSet_Script_Tangsa, gxCharSet_Script_Tangsa },
	{ "Tangut", mxCharSet_Script_Tangut, gxCharSet_Script_Tangut },
	{ "Tavt", mxCharSet_Script_Tai_Viet, gxCharSet_Script_Tai_Viet },
	{ "Telu", mxCharSet_Script_Telugu, gxCharSet_Script_Telugu },
	{ "Telugu", mxCharSet_Script_Telugu, gxCharSet_Script_Telugu },
	{ "Tfng", mxCharSet_Script_Tifinagh, gxCharSet_Script_Tifinagh },
	{ "Tglg", mxCharSet_Script_Tagalog, gxCharSet_Script_Tagalog },
	{ "Thaa", mxCharSet_Script_Thaana, gxCharSet_Script_Thaana },
	{ "Thaana", mxCharSet_Script_Thaana, gxCharSet_Script_Thaana },
	{ "Thai", mxCharSet_Script_Thai, gxCharSet_Script_Thai },
	{ "Tibetan", mxCharSet_Script_Tibetan, gxCharSet_Script_Tibetan },
	{ "Tibt", mxCharSet_Script_Tibetan, gxCharSet_Script_Tibetan },
	{ "Tifinagh", mxCharSet_Script_Tifinagh, gxCharSet_Script_Tifinagh },
	{ "Tirh", mxCharSet_Script_Tirhuta, gxCharSet_Script_Tirhuta },
	{ "Tirhuta", mxCharSet_Script_Tirhuta, gxCharSet_Script_Tirhuta },
	{ "Tnsa", mxCharSet_Script_Tangsa, gxCharSet_Script_Tangsa },
	{ "Toto", mxCharSet_Script_Toto, gxCharSet_Script_Toto },
	{ "Ugar", mxCharSet_Script_Ugaritic, gxCharSet_Script_Ugaritic },
	{ "Ugaritic", mxCharSet_Script_Ugaritic, gxCharSet_Script_Ugaritic },
	{ "Vai", mxCharSet_Script_Vai, gxCharSet_Script_Vai },
	{ "Vaii", mxCharSet_Script_Vai, gxCharSet_Script_Vai },
	{ "Vith", mxCharSet_Script_Vithkuqi, gxCharSet_Script_Vithkuqi },
	{ "Vithkuqi", mxCharSet_Script_Vithkuqi, gxCharSet_Script_Vithkuqi },
	{ "Wancho", mxCharSet_Script_Wancho, gxCharSet_Script_Wancho },
	{ "Wara", mxCharSet_Script_Warang_Citi, gxCharSet_Script_Warang_Citi },
	{ "Warang_Citi", mxCharSet_Script_Warang_Citi, gxCharSet_Script_Warang_Citi },
	{ "Wcho", mxCharSet_Script_Wancho, gxCharSet_Script_Wancho },
	{ "Xpeo", mxCharSet_Script_Old_Persian, gxCharSet_Script_Old_Persian },
	{ "Xsux", mxCharSet_Script_Cuneiform, gxCharSet_Script_Cuneiform },
	{ "Yezi", mxCharSet_Script_Yezidi, gxCharSet_Script_Yezidi },
	{ "Yezidi", mxCharSet_Script_Yezidi, gxCharSet_Script_Yezidi },
	{ "Yi", mxCharSet_Script_Yi, gxCharSet_Script_Yi },
	{ "Yiii", mxCharSet_Script_Yi, gxCharSet_Script_Yi },
	{ "Zanabazar_Square", mxCharSet_Script_Zanabazar_Square, gxCharSet_Script_Zanabazar_Square },
	{ "Zanb", mxCharSet_Script_Zanabazar_Square, gxCharSet_Script_Zanabazar_Square },
	{ "Zinh", mxCharSet_Script_Inherited, gxCharSet_Script_Inherited },
	{ "Zyyy", mxCharSet_Script_Common, gxCharSet_Script_Common },
};
#define mxCharSet_Script_Extensions 320
static const txCharSetUnicodeProperty ICACHE_RODATA_ATTR gxCharSet_Script_Extensions[mxCharSet_Script_Extensions] = {
	{ "Adlam", mxCharSet_Script_Extensions_Adlam, gxCharSet_Script_Extensions_Adlam },
	{ "Adlm", mxCharSet_Script_Extensions_Adlam, gxCharSet_Script_Extensions_Adlam },
	{ "Aghb", mxCharSet_Script_Extensions_Caucasian_Albanian, gxCharSet_Script_Extensions_Caucasian_Albanian },
	{ "Ahom", mxCharSet_Script_Extensions_Ahom, gxCharSet_Script_Extensions_Ahom },
	{ "Anatolian_Hieroglyphs", mxCharSet_Script_Extensions_Anatolian_Hieroglyphs, gxCharSet_Script_Extensions_Anatolian_Hieroglyphs },
	{ "Arab", mxCharSet_Script_Extensions_Arabic, gxCharSet_Script_Extensions_Arabic },
	{ "Arabic", mxCharSet_Script_Extensions_Arabic, gxCharSet_Script_Extensions_Arabic },
	{ "Armenian", mxCharSet_Script_Extensions_Armenian, gxCharSet_Script_Extensions_Armenian },
	{ "Armi", mxCharSet_Script_Extensions_Imperial_Aramaic, gxCharSet_Script_Extensions_Imperial_Aramaic },
	{ "Armn", mxCharSet_Script_Extensions_Armenian, gxCharSet_Script_Extensions_Armenian },
	{ "Avestan", mxCharSet_Script_Extensions_Avestan, gxCharSet_Script_Extensions_Avestan },
	{ "Avst", mxCharSet_Script_Extensions_Avestan, gxCharSet_Script_Extensions_Avestan },
	{ "Bali", mxCharSet_Script_Extensions_Balinese, gxCharSet_Script_Extensions_Balinese },
	{ "Balinese", mxCharSet_Script_Extensions_Balinese, gxCharSet_Script_Extensions_Balinese },
	{ "Bamu", mxCharSet_Script_Extensions_Bamum, gxCharSet_Script_Extensions_Bamum },
	{ "Bamum", mxCharSet_Script_Extensions_Bamum, gxCharSet_Script_Extensions_Bamum },
	{ "Bass", mxCharSet_Script_Extensions_Bassa_Vah, gxCharSet_Script_Extensions_Bassa_Vah },
	{ "Bassa_Vah", mxCharSet_Script_Extensions_Bassa_Vah, gxCharSet_Script_Extensions_Bassa_Vah },
	{ "Batak", mxCharSet_Script_Extensions_Batak, gxCharSet_Script_Extensions_Batak },
	{ "Batk", mxCharSet_Script_Extensions_Batak, gxCharSet_Script_Extensions_Batak },
	{ "Beng", mxCharSet_Script_Extensions_Bengali, gxCharSet_Script_Extensions_Bengali },
	{ "Bengali", mxCharSet_Script_Extensions_Bengali, gxCharSet_Script_Extensions_Bengali },
	{ "Bhaiksuki", mxCharSet_Script_Extensions_Bhaiksuki, gxCharSet_Script_Extensions_Bhaiksuki },
	{ "Bhks", mxCharSet_Script_Extensions_Bhaiksuki, gxCharSet_Script_Extensions_Bhaiksuki },
	{ "Bopo", mxCharSet_Script_Extensions_Bopomofo, gxCharSet_Script_Extensions_Bopomofo },
	{ "Bopomofo", mxCharSet_Script_Extensions_Bopomofo, gxCharSet_Script_Extensions_Bopomofo },
	{ "Brah", mxCharSet_Script_Extensions_Brahmi, gxCharSet_Script_Extensions_Brahmi },
	{ "Brahmi", mxCharSet_Script_Extensions_Brahmi, gxCharSet_Script_Extensions_Brahmi },
	{ "Brai", mxCharSet_Script_Extensions_Braille, gxCharSet_Script_Extensions_Braille },
	{ "Braille", mxCharSet_Script_Extensions_Braille, gxCharSet_Script_Extensions_Braille },
	{ "Bugi", mxCharSet_Script_Extensions_Buginese, gxCharSet_Script_Extensions_Buginese },
	{ "Buginese", mxCharSet_Script_Extensions_Buginese, gxCharSet_Script_Extensions_Buginese },
	{ "Buhd", mxCharSet_Script_Extensions_Buhid, gxCharSet_Script_Extensions_Buhid },
	{ "Buhid", mxCharSet_Script_Extensions_Buhid, gxCharSet_Script_Extensions_Buhid },
	{ "Cakm", mxCharSet_Script_Extensions_Chakma, gxCharSet_Script_Extensions_Chakma },
	{ "Canadian_Aboriginal", mxCharSet_Script_Extensions_Canadian_Aboriginal, gxCharSet_Script_Extensions_Canadian_Aboriginal },
	{ "Cans", mxCharSet_Script_Extensions_Canadian_Aboriginal, gxCharSet_Script_Extensions_Canadian_Aboriginal },
	{ "Cari", mxCharSet_Script_Extensions_Carian, gxCharSet_Script_Extensions_Carian },
	{ "Carian", mxCharSet_Script_Extensions_Carian, gxCharSet_Script_Extensions_Carian },
	{ "Caucasian_Albanian", mxCharSet_Script_Extensions_Caucasian_Albanian, gxCharSet_Script_Extensions_Caucasian_Albanian },
	{ "Chakma", mxCharSet_Script_Extensions_Chakma, gxCharSet_Script_Extensions_Chakma },
	{ "Cham", mxCharSet_Script_Extensions_Cham, gxCharSet_Script_Extensions_Cham },
	{ "Cher", mxCharSet_Script_Extensions_Cherokee, gxCharSet_Script_Extensions_Cherokee },
	{ "Cherokee", mxCharSet_Script_Extensions_Cherokee, gxCharSet_Script_Extensions_Cherokee },
	{ "Chorasmian", mxCharSet_Script_Extensions_Chorasmian, gxCharSet_Script_Extensions_Chorasmian },
	{ "Chrs", mxCharSet_Script_Extensions_Chorasmian, gxCharSet_Script_Extensions_Chorasmian },
	{ "Common", mxCharSet_Script_Extensions_Common, gxCharSet_Script_Extensions_Common },
	{ "Copt", mxCharSet_Script_Extensions_Coptic, gxCharSet_Script_Extensions_Coptic },
	{ "Coptic", mxCharSet_Script_Extensions_Coptic, gxCharSet_Script_Extensions_Coptic },
	{ "Cpmn", mxCharSet_Script_Extensions_Cypro_Minoan, gxCharSet_Script_Extensions_Cypro_Minoan },
	{ "Cprt", mxCharSet_Script_Extensions_Cypriot, gxCharSet_Script_Extensions_Cypriot },
	{ "Cuneiform", mxCharSet_Script_Extensions_Cuneiform, gxCharSet_Script_Extensions_Cuneiform },
	{ "Cypriot", mxCharSet_Script_Extensions_Cypriot, gxCharSet_Script_Extensions_Cypriot },
	{ "Cypro_Minoan", mxCharSet_Script_Extensions_Cypro_Minoan, gxCharSet_Script_Extensions_Cypro_Minoan },
	{ "Cyrillic", mxCharSet_Script_Extensions_Cyrillic, gxCharSet_Script_Extensions_Cyrillic },
	{ "Cyrl", mxCharSet_Script_Extensions_Cyrillic, gxCharSet_Script_Extensions_Cyrillic },
	{ "Deseret", mxCharSet_Script_Extensions_Deseret, gxCharSet_Script_Extensions_Deseret },
	{ "Deva", mxCharSet_Script_Extensions_Devanagari, gxCharSet_Script_Extensions_Devanagari },
	{ "Devanagari", mxCharSet_Script_Extensions_Devanagari, gxCharSet_Script_Extensions_Devanagari },
	{ "Diak", mxCharSet_Script_Extensions_Dives_Akuru, gxCharSet_Script_Extensions_Dives_Akuru },
	{ "Dives_Akuru", mxCharSet_Script_Extensions_Dives_Akuru, gxCharSet_Script_Extensions_Dives_Akuru },
	{ "Dogr", mxCharSet_Script_Extensions_Dogra, gxCharSet_Script_Extensions_Dogra },
	{ "Dogra", mxCharSet_Script_Extensions_Dogra, gxCharSet_Script_Extensions_Dogra },
	{ "Dsrt", mxCharSet_Script_Extensions_Deseret, gxCharSet_Script_Extensions_Deseret },
	{ "Dupl", mxCharSet_Script_Extensions_Duployan, gxCharSet_Script_Extensions_Duployan },
	{ "Duployan", mxCharSet_Script_Extensions_Duployan, gxCharSet_Script_Extensions_Duployan },
	{ "Egyp", mxCharSet_Script_Extensions_Egyptian_Hieroglyphs, gxCharSet_Script_Extensions_Egyptian_Hieroglyphs },
	{ "Egyptian_Hieroglyphs", mxCharSet_Script_Extensions_Egyptian_Hieroglyphs, gxCharSet_Script_Extensions_Egyptian_Hieroglyphs },
	{ "Elba", mxCharSet_Script_Extensions_Elbasan, gxCharSet_Script_Extensions_Elbasan },
	{ "Elbasan", mxCharSet_Script_Extensions_Elbasan, gxCharSet_Script_Extensions_Elbasan },
	{ "Elym", mxCharSet_Script_Extensions_Elymaic, gxCharSet_Script_Extensions_Elymaic },
	{ "Elymaic", mxCharSet_Script_Extensions_Elymaic, gxCharSet_Script_Extensions_Elymaic },
	{ "Ethi", mxCharSet_Script_Extensions_Ethiopic, gxCharSet_Script_Extensions_Ethiopic },
	{ "Ethiopic", mxCharSet_Script_Extensions_Ethiopic, gxCharSet_Script_Extensions_Ethiopic },
	{ "Geor", mxCharSet_Script_Extensions_Georgian, gxCharSet_Script_Extensions_Georgian },
	{ "Georgian", mxCharSet_Script_Extensions_Georgian, gxCharSet_Script_Extensions_Georgian },
	{ "Glag", mxCharSet_Script_Extensions_Glagolitic, gxCharSet_Script_Extensions_Glagolitic },
	{ "Glagolitic", mxCharSet_Script_Extensions_Glagolitic, gxCharSet_Script_Extensions_Glagolitic },
	{ "Gong", mxCharSet_Script_Extensions_Gunjala_Gondi, gxCharSet_Script_Extensions_Gunjala_Gondi },
	{ "Gonm", mxCharSet_Script_Extensions_Masaram_Gondi, gxCharSet_Script_Extensions_Masaram_Gondi },
	{ "Goth", mxCharSet_Script_Extensions_Gothic, gxCharSet_Script_Extensions_Gothic },
	{ "Gothic", mxCharSet_Script_Extensions_Gothic, gxCharSet_Script_Extensions_Gothic },
	{ "Gran", mxCharSet_Script_Extensions_Grantha, gxCharSet_Script_Extensions_Grantha },
	{ "Grantha", mxCharSet_Script_Extensions_Grantha, gxCharSet_Script_Extensions_Grantha },
	{ "Greek", mxCharSet_Script_Extensions_Greek, gxCharSet_Script_Extensions_Greek },
	{ "Grek", mxCharSet_Script_Extensions_Greek, gxCharSet_Script_Extensions_Greek },
	{ "Gujarati", mxCharSet_Script_Extensions_Gujarati, gxCharSet_Script_Extensions_Gujarati },
	{ "Gujr", mxCharSet_Script_Extensions_Gujarati, gxCharSet_Script_Extensions_Gujarati },
	{ "Gunjala_Gondi", mxCharSet_Script_Extensions_Gunjala_Gondi, gxCharSet_Script_Extensions_Gunjala_Gondi },
	{ "Gurmukhi", mxCharSet_Script_Extensions_Gurmukhi, gxCharSet_Script_Extensions_Gurmukhi },
	{ "Guru", mxCharSet_Script_Extensions_Gurmukhi, gxCharSet_Script_Extensions_Gurmukhi },
	{ "Han", mxCharSet_Script_Extensions_Han, gxCharSet_Script_Extensions_Han },
	{ "Hang", mxCharSet_Script_Extensions_Hangul, gxCharSet_Script_Extensions_Hangul },
	{ "Hangul", mxCharSet_Script_Extensions_Hangul, gxCharSet_Script_Extensions_Hangul },
	{ "Hani", mxCharSet_Script_Extensions_Han, gxCharSet_Script_Extensions_Han },
	{ "Hanifi_Rohingya", mxCharSet_Script_Extensions_Hanifi_Rohingya, gxCharSet_Script_Extensions_Hanifi_Rohingya },
	{ "Hano", mxCharSet_Script_Extensions_Hanunoo, gxCharSet_Script_Extensions_Hanunoo },
	{ "Hanunoo", mxCharSet_Script_Extensions_Hanunoo, gxCharSet_Script_Extensions_Hanunoo },
	{ "Hatr", mxCharSet_Script_Extensions_Hatran, gxCharSet_Script_Extensions_Hatran },
	{ "Hatran", mxCharSet_Script_Extensions_Hatran, gxCharSet_Script_Extensions_Hatran },
	{ "Hebr", mxCharSet_Script_Extensions_Hebrew, gxCharSet_Script_Extensions_Hebrew },
	{ "Hebrew", mxCharSet_Script_Extensions_Hebrew, gxCharSet_Script_Extensions_Hebrew },
	{ "Hira", mxCharSet_Script_Extensions_Hiragana, gxCharSet_Script_Extensions_Hiragana },
	{ "Hiragana", mxCharSet_Script_Extensions_Hiragana, gxCharSet_Script_Extensions_Hiragana },
	{ "Hluw", mxCharSet_Script_Extensions_Anatolian_Hieroglyphs, gxCharSet_Script_Extensions_Anatolian_Hieroglyphs },
	{ "Hmng", mxCharSet_Script_Extensions_Pahawh_Hmong, gxCharSet_Script_Extensions_Pahawh_Hmong },
	{ "Hmnp", mxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong, gxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong },
	{ "Hung", mxCharSet_Script_Extensions_Old_Hungarian, gxCharSet_Script_Extensions_Old_Hungarian },
	{ "Imperial_Aramaic", mxCharSet_Script_Extensions_Imperial_Aramaic, gxCharSet_Script_Extensions_Imperial_Aramaic },
	{ "Inherited", mxCharSet_Script_Extensions_Inherited, gxCharSet_Script_Extensions_Inherited },
	{ "Inscriptional_Pahlavi", mxCharSet_Script_Extensions_Inscriptional_Pahlavi, gxCharSet_Script_Extensions_Inscriptional_Pahlavi },
	{ "Inscriptional_Parthian", mxCharSet_Script_Extensions_Inscriptional_Parthian, gxCharSet_Script_Extensions_Inscriptional_Parthian },
	{ "Ital", mxCharSet_Script_Extensions_Old_Italic, gxCharSet_Script_Extensions_Old_Italic },
	{ "Java", mxCharSet_Script_Extensions_Javanese, gxCharSet_Script_Extensions_Javanese },
	{ "Javanese", mxCharSet_Script_Extensions_Javanese, gxCharSet_Script_Extensions_Javanese },
	{ "Kaithi", mxCharSet_Script_Extensions_Kaithi, gxCharSet_Script_Extensions_Kaithi },
	{ "Kali", mxCharSet_Script_Extensions_Kayah_Li, gxCharSet_Script_Extensions_Kayah_Li },
	{ "Kana", mxCharSet_Script_Extensions_Katakana, gxCharSet_Script_Extensions_Katakana },
	{ "Kannada", mxCharSet_Script_Extensions_Kannada, gxCharSet_Script_Extensions_Kannada },
	{ "Katakana", mxCharSet_Script_Extensions_Katakana, gxCharSet_Script_Extensions_Katakana },
	{ "Kawi", mxCharSet_Script_Extensions_Kawi, gxCharSet_Script_Extensions_Kawi },
	{ "Kayah_Li", mxCharSet_Script_Extensions_Kayah_Li, gxCharSet_Script_Extensions_Kayah_Li },
	{ "Khar", mxCharSet_Script_Extensions_Kharoshthi, gxCharSet_Script_Extensions_Kharoshthi },
	{ "Kharoshthi", mxCharSet_Script_Extensions_Kharoshthi, gxCharSet_Script_Extensions_Kharoshthi },
	{ "Khitan_Small_Script", mxCharSet_Script_Extensions_Khitan_Small_Script, gxCharSet_Script_Extensions_Khitan_Small_Script },
	{ "Khmer", mxCharSet_Script_Extensions_Khmer, gxCharSet_Script_Extensions_Khmer },
	{ "Khmr", mxCharSet_Script_Extensions_Khmer, gxCharSet_Script_Extensions_Khmer },
	{ "Khoj", mxCharSet_Script_Extensions_Khojki, gxCharSet_Script_Extensions_Khojki },
	{ "Khojki", mxCharSet_Script_Extensions_Khojki, gxCharSet_Script_Extensions_Khojki },
	{ "Khudawadi", mxCharSet_Script_Extensions_Khudawadi, gxCharSet_Script_Extensions_Khudawadi },
	{ "Kits", mxCharSet_Script_Extensions_Khitan_Small_Script, gxCharSet_Script_Extensions_Khitan_Small_Script },
	{ "Knda", mxCharSet_Script_Extensions_Kannada, gxCharSet_Script_Extensions_Kannada },
	{ "Kthi", mxCharSet_Script_Extensions_Kaithi, gxCharSet_Script_Extensions_Kaithi },
	{ "Lana", mxCharSet_Script_Extensions_Tai_Tham, gxCharSet_Script_Extensions_Tai_Tham },
	{ "Lao", mxCharSet_Script_Extensions_Lao, gxCharSet_Script_Extensions_Lao },
	{ "Laoo", mxCharSet_Script_Extensions_Lao, gxCharSet_Script_Extensions_Lao },
	{ "Latin", mxCharSet_Script_Extensions_Latin, gxCharSet_Script_Extensions_Latin },
	{ "Latn", mxCharSet_Script_Extensions_Latin, gxCharSet_Script_Extensions_Latin },
	{ "Lepc", mxCharSet_Script_Extensions_Lepcha, gxCharSet_Script_Extensions_Lepcha },
	{ "Lepcha", mxCharSet_Script_Extensions_Lepcha, gxCharSet_Script_Extensions_Lepcha },
	{ "Limb", mxCharSet_Script_Extensions_Limbu, gxCharSet_Script_Extensions_Limbu },
	{ "Limbu", mxCharSet_Script_Extensions_Limbu, gxCharSet_Script_Extensions_Limbu },
	{ "Lina", mxCharSet_Script_Extensions_Linear_A, gxCharSet_Script_Extensions_Linear_A },
	{ "Linb", mxCharSet_Script_Extensions_Linear_B, gxCharSet_Script_Extensions_Linear_B },
	{ "Linear_A", mxCharSet_Script_Extensions_Linear_A, gxCharSet_Script_Extensions_Linear_A },
	{ "Linear_B", mxCharSet_Script_Extensions_Linear_B, gxCharSet_Script_Extensions_Linear_B },
	{ "Lisu", mxCharSet_Script_Extensions_Lisu, gxCharSet_Script_Extensions_Lisu },
	{ "Lyci", mxCharSet_Script_Extensions_Lycian, gxCharSet_Script_Extensions_Lycian },
	{ "Lycian", mxCharSet_Script_Extensions_Lycian, gxCharSet_Script_Extensions_Lycian },
	{ "Lydi", mxCharSet_Script_Extensions_Lydian, gxCharSet_Script_Extensions_Lydian },
	{ "Lydian", mxCharSet_Script_Extensions_Lydian, gxCharSet_Script_Extensions_Lydian },
	{ "Mahajani", mxCharSet_Script_Extensions_Mahajani, gxCharSet_Script_Extensions_Mahajani },
	{ "Mahj", mxCharSet_Script_Extensions_Mahajani, gxCharSet_Script_Extensions_Mahajani },
	{ "Maka", mxCharSet_Script_Extensions_Makasar, gxCharSet_Script_Extensions_Makasar },
	{ "Makasar", mxCharSet_Script_Extensions_Makasar, gxCharSet_Script_Extensions_Makasar },
	{ "Malayalam", mxCharSet_Script_Extensions_Malayalam, gxCharSet_Script_Extensions_Malayalam },
	{ "Mand", mxCharSet_Script_Extensions_Mandaic, gxCharSet_Script_Extensions_Mandaic },
	{ "Mandaic", mxCharSet_Script_Extensions_Mandaic, gxCharSet_Script_Extensions_Mandaic },
	{ "Mani", mxCharSet_Script_Extensions_Manichaean, gxCharSet_Script_Extensions_Manichaean },
	{ "Manichaean", mxCharSet_Script_Extensions_Manichaean, gxCharSet_Script_Extensions_Manichaean },
	{ "Marc", mxCharSet_Script_Extensions_Marchen, gxCharSet_Script_Extensions_Marchen },
	{ "Marchen", mxCharSet_Script_Extensions_Marchen, gxCharSet_Script_Extensions_Marchen },
	{ "Masaram_Gondi", mxCharSet_Script_Extensions_Masaram_Gondi, gxCharSet_Script_Extensions_Masaram_Gondi },
	{ "Medefaidrin", mxCharSet_Script_Extensions_Medefaidrin, gxCharSet_Script_Extensions_Medefaidrin },
	{ "Medf", mxCharSet_Script_Extensions_Medefaidrin, gxCharSet_Script_Extensions_Medefaidrin },
	{ "Meetei_Mayek", mxCharSet_Script_Extensions_Meetei_Mayek, gxCharSet_Script_Extensions_Meetei_Mayek },
	{ "Mend", mxCharSet_Script_Extensions_Mende_Kikakui, gxCharSet_Script_Extensions_Mende_Kikakui },
	{ "Mende_Kikakui", mxCharSet_Script_Extensions_Mende_Kikakui, gxCharSet_Script_Extensions_Mende_Kikakui },
	{ "Merc", mxCharSet_Script_Extensions_Meroitic_Cursive, gxCharSet_Script_Extensions_Meroitic_Cursive },
	{ "Mero", mxCharSet_Script_Extensions_Meroitic_Hieroglyphs, gxCharSet_Script_Extensions_Meroitic_Hieroglyphs },
	{ "Meroitic_Cursive", mxCharSet_Script_Extensions_Meroitic_Cursive, gxCharSet_Script_Extensions_Meroitic_Cursive },
	{ "Meroitic_Hieroglyphs", mxCharSet_Script_Extensions_Meroitic_Hieroglyphs, gxCharSet_Script_Extensions_Meroitic_Hieroglyphs },
	{ "Miao", mxCharSet_Script_Extensions_Miao, gxCharSet_Script_Extensions_Miao },
	{ "Mlym", mxCharSet_Script_Extensions_Malayalam, gxCharSet_Script_Extensions_Malayalam },
	{ "Modi", mxCharSet_Script_Extensions_Modi, gxCharSet_Script_Extensions_Modi },
	{ "Mong", mxCharSet_Script_Extensions_Mongolian, gxCharSet_Script_Extensions_Mongolian },
	{ "Mongolian", mxCharSet_Script_Extensions_Mongolian, gxCharSet_Script_Extensions_Mongolian },
	{ "Mro", mxCharSet_Script_Extensions_Mro, gxCharSet_Script_Extensions_Mro },
	{ "Mroo", mxCharSet_Script_Extensions_Mro, gxCharSet_Script_Extensions_Mro },
	{ "Mtei", mxCharSet_Script_Extensions_Meetei_Mayek, gxCharSet_Script_Extensions_Meetei_Mayek },
	{ "Mult", mxCharSet_Script_Extensions_Multani, gxCharSet_Script_Extensions_Multani },
	{ "Multani", mxCharSet_Script_Extensions_Multani, gxCharSet_Script_Extensions_Multani },
	{ "Myanmar", mxCharSet_Script_Extensions_Myanmar, gxCharSet_Script_Extensions_Myanmar },
	{ "Mymr", mxCharSet_Script_Extensions_Myanmar, gxCharSet_Script_Extensions_Myanmar },
	{ "Nabataean", mxCharSet_Script_Extensions_Nabataean, gxCharSet_Script_Extensions_Nabataean },
	{ "Nag_Mundari", mxCharSet_Script_Extensions_Nag_Mundari, gxCharSet_Script_Extensions_Nag_Mundari },
	{ "Nagm", mxCharSet_Script_Extensions_Nag_Mundari, gxCharSet_Script_Extensions_Nag_Mundari },
	{ "Nand", mxCharSet_Script_Extensions_Nandinagari, gxCharSet_Script_Extensions_Nandinagari },
	{ "Nandinagari", mxCharSet_Script_Extensions_Nandinagari, gxCharSet_Script_Extensions_Nandinagari },
	{ "Narb", mxCharSet_Script_Extensions_Old_North_Arabian, gxCharSet_Script_Extensions_Old_North_Arabian },
	{ "Nbat", mxCharSet_Script_Extensions_Nabataean, gxCharSet_Script_Extensions_Nabataean },
	{ "New_Tai_Lue", mxCharSet_Script_Extensions_New_Tai_Lue, gxCharSet_Script_Extensions_New_Tai_Lue },
	{ "Newa", mxCharSet_Script_Extensions_Newa, gxCharSet_Script_Extensions_Newa },
	{ "Nko", mxCharSet_Script_Extensions_Nko, gxCharSet_Script_Extensions_Nko },
	{ "Nkoo", mxCharSet_Script_Extensions_Nko, gxCharSet_Script_Extensions_Nko },
	{ "Nshu", mxCharSet_Script_Extensions_Nushu, gxCharSet_Script_Extensions_Nushu },
	{ "Nushu", mxCharSet_Script_Extensions_Nushu, gxCharSet_Script_Extensions_Nushu },
	{ "Nyiakeng_Puachue_Hmong", mxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong, gxCharSet_Script_Extensions_Nyiakeng_Puachue_Hmong },
	{ "Ogam", mxCharSet_Script_Extensions_Ogham, gxCharSet_Script_Extensions_Ogham },
	{ "Ogham", mxCharSet_Script_Extensions_Ogham, gxCharSet_Script_Extensions_Ogham },
	{ "Ol_Chiki", mxCharSet_Script_Extensions_Ol_Chiki, gxCharSet_Script_Extensions_Ol_Chiki },
	{ "Olck", mxCharSet_Script_Extensions_Ol_Chiki, gxCharSet_Script_Extensions_Ol_Chiki },
	{ "Old_Hungarian", mxCharSet_Script_Extensions_Old_Hungarian, gxCharSet_Script_Extensions_Old_Hungarian },
	{ "Old_Italic", mxCharSet_Script_Extensions_Old_Italic, gxCharSet_Script_Extensions_Old_Italic },
	{ "Old_North_Arabian", mxCharSet_Script_Extensions_Old_North_Arabian, gxCharSet_Script_Extensions_Old_North_Arabian },
	{ "Old_Permic", mxCharSet_Script_Extensions_Old_Permic, gxCharSet_Script_Extensions_Old_Permic },
	{ "Old_Persian", mxCharSet_Script_Extensions_Old_Persian, gxCharSet_Script_Extensions_Old_Persian },
	{ "Old_Sogdian", mxCharSet_Script_Extensions_Old_Sogdian, gxCharSet_Script_Extensions_Old_Sogdian },
	{ "Old_South_Arabian", mxCharSet_Script_Extensions_Old_South_Arabian, gxCharSet_Script_Extensions_Old_South_Arabian },
	{ "Old_Turkic", mxCharSet_Script_Extensions_Old_Turkic, gxCharSet_Script_Extensions_Old_Turkic },
	{ "Old_Uyghur", mxCharSet_Script_Extensions_Old_Uyghur, gxCharSet_Script_Extensions_Old_Uyghur },
	{ "Oriya", mxCharSet_Script_Extensions_Oriya, gxCharSet_Script_Extensions_Oriya },
	{ "Orkh", mxCharSet_Script_Extensions_Old_Turkic, gxCharSet_Script_Extensions_Old_Turkic },
	{ "Orya", mxCharSet_Script_Extensions_Oriya, gxCharSet_Script_Extensions_Oriya },
	{ "Osage", mxCharSet_Script_Extensions_Osage, gxCharSet_Script_Extensions_Osage },
	{ "Osge", mxCharSet_Script_Extensions_Osage, gxCharSet_Script_Extensions_Osage },
	{ "Osma", mxCharSet_Script_Extensions_Osmanya, gxCharSet_Script_Extensions_Osmanya },
	{ "Osmanya", mxCharSet_Script_Extensions_Osmanya, gxCharSet_Script_Extensions_Osmanya },
	{ "Ougr", mxCharSet_Script_Extensions_Old_Uyghur, gxCharSet_Script_Extensions_Old_Uyghur },
	{ "Pahawh_Hmong", mxCharSet_Script_Extensions_Pahawh_Hmong, gxCharSet_Script_Extensions_Pahawh_Hmong },
	{ "Palm", mxCharSet_Script_Extensions_Palmyrene, gxCharSet_Script_Extensions_Palmyrene },
	{ "Palmyrene", mxCharSet_Script_Extensions_Palmyrene, gxCharSet_Script_Extensions_Palmyrene },
	{ "Pau_Cin_Hau", mxCharSet_Script_Extensions_Pau_Cin_Hau, gxCharSet_Script_Extensions_Pau_Cin_Hau },
	{ "Pauc", mxCharSet_Script_Extensions_Pau_Cin_Hau, gxCharSet_Script_Extensions_Pau_Cin_Hau },
	{ "Perm", mxCharSet_Script_Extensions_Old_Permic, gxCharSet_Script_Extensions_Old_Permic },
	{ "Phag", mxCharSet_Script_Extensions_Phags_Pa, gxCharSet_Script_Extensions_Phags_Pa },
	{ "Phags_Pa", mxCharSet_Script_Extensions_Phags_Pa, gxCharSet_Script_Extensions_Phags_Pa },
	{ "Phli", mxCharSet_Script_Extensions_Inscriptional_Pahlavi, gxCharSet_Script_Extensions_Inscriptional_Pahlavi },
	{ "Phlp", mxCharSet_Script_Extensions_Psalter_Pahlavi, gxCharSet_Script_Extensions_Psalter_Pahlavi },
	{ "Phnx", mxCharSet_Script_Extensions_Phoenician, gxCharSet_Script_Extensions_Phoenician },
	{ "Phoenician", mxCharSet_Script_Extensions_Phoenician, gxCharSet_Script_Extensions_Phoenician },
	{ "Plrd", mxCharSet_Script_Extensions_Miao, gxCharSet_Script_Extensions_Miao },
	{ "Prti", mxCharSet_Script_Extensions_Inscriptional_Parthian, gxCharSet_Script_Extensions_Inscriptional_Parthian },
	{ "Psalter_Pahlavi", mxCharSet_Script_Extensions_Psalter_Pahlavi, gxCharSet_Script_Extensions_Psalter_Pahlavi },
	{ "Qaac", mxCharSet_Script_Extensions_Coptic, gxCharSet_Script_Extensions_Coptic },
	{ "Qaai", mxCharSet_Script_Extensions_Inherited, gxCharSet_Script_Extensions_Inherited },
	{ "Rejang", mxCharSet_Script_Extensions_Rejang, gxCharSet_Script_Extensions_Rejang },
	{ "Rjng", mxCharSet_Script_Extensions_Rejang, gxCharSet_Script_Extensions_Rejang },
	{ "Rohg", mxCharSet_Script_Extensions_Hanifi_Rohingya, gxCharSet_Script_Extensions_Hanifi_Rohingya },
	{ "Runic", mxCharSet_Script_Extensions_Runic, gxCharSet_Script_Extensions_Runic },
	{ "Runr", mxCharSet_Script_Extensions_Runic, gxCharSet_Script_Extensions_Runic },
	{ "Samaritan", mxCharSet_Script_Extensions_Samaritan, gxCharSet_Script_Extensions_Samaritan },
	{ "Samr", mxCharSet_Script_Extensions_Samaritan, gxCharSet_Script_Extensions_Samaritan },
	{ "Sarb", mxCharSet_Script_Extensions_Old_South_Arabian, gxCharSet_Script_Extensions_Old_South_Arabian },
	{ "Saur", mxCharSet_Script_Extensions_Saurashtra, gxCharSet_Script_Extensions_Saurashtra },
	{ "Saurashtra", mxCharSet_Script_Extensions_Saurashtra, gxCharSet_Script_Extensions_Saurashtra },
	{ "Sgnw", mxCharSet_Script_Extensions_SignWriting, gxCharSet_Script_Extensions_SignWriting },
	{ "Sharada", mxCharSet_Script_Extensions_Sharada, gxCharSet_Script_Extensions_Sharada },
	{ "Shavian", mxCharSet_Script_Extensions_Shavian, gxCharSet_Script_Extensions_Shavian },
	{ "Shaw", mxCharSet_Script_Extensions_Shavian, gxCharSet_Script_Extensions_Shavian },
	{ "Shrd", mxCharSet_Script_Extensions_Sharada, gxCharSet_Script_Extensions_Sharada },
	{ "Sidd", mxCharSet_Script_Extensions_Siddham, gxCharSet_Script_Extensions_Siddham },
	{ "Siddham", mxCharSet_Script_Extensions_Siddham, gxCharSet_Script_Extensions_Siddham },
	{ "SignWriting", mxCharSet_Script_Extensions_SignWriting, gxCharSet_Script_Extensions_SignWriting },
	{ "Sind", mxCharSet_Script_Extensions_Khudawadi, gxCharSet_Script_Extensions_Khudawadi },
	{ "Sinh", mxCharSet_Script_Extensions_Sinhala, gxCharSet_Script_Extensions_Sinhala },
	{ "Sinhala", mxCharSet_Script_Extensions_Sinhala, gxCharSet_Script_Extensions_Sinhala },
	{ "Sogd", mxCharSet_Script_Extensions_Sogdian, gxCharSet_Script_Extensions_Sogdian },
	{ "Sogdian", mxCharSet_Script_Extensions_Sogdian, gxCharSet_Script_Extensions_Sogdian },
	{ "Sogo", mxCharSet_Script_Extensions_Old_Sogdian, gxCharSet_Script_Extensions_Old_Sogdian },
	{ "Sora", mxCharSet_Script_Extensions_Sora_Sompeng, gxCharSet_Script_Extensions_Sora_Sompeng },
	{ "Sora_Sompeng", mxCharSet_Script_Extensions_Sora_Sompeng, gxCharSet_Script_Extensions_Sora_Sompeng },
	{ "Soyo", mxCharSet_Script_Extensions_Soyombo, gxCharSet_Script_Extensions_Soyombo },
	{ "Soyombo", mxCharSet_Script_Extensions_Soyombo, gxCharSet_Script_Extensions_Soyombo },
	{ "Sund", mxCharSet_Script_Extensions_Sundanese, gxCharSet_Script_Extensions_Sundanese },
	{ "Sundanese", mxCharSet_Script_Extensions_Sundanese, gxCharSet_Script_Extensions_Sundanese },
	{ "Sylo", mxCharSet_Script_Extensions_Syloti_Nagri, gxCharSet_Script_Extensions_Syloti_Nagri },
	{ "Syloti_Nagri", mxCharSet_Script_Extensions_Syloti_Nagri, gxCharSet_Script_Extensions_Syloti_Nagri },
	{ "Syrc", mxCharSet_Script_Extensions_Syriac, gxCharSet_Script_Extensions_Syriac },
	{ "Syriac", mxCharSet_Script_Extensions_Syriac, gxCharSet_Script_Extensions_Syriac },
	{ "Tagalog", mxCharSet_Script_Extensions_Tagalog, gxCharSet_Script_Extensions_Tagalog },
	{ "Tagb", mxCharSet_Script_Extensions_Tagbanwa, gxCharSet_Script_Extensions_Tagbanwa },
	{ "Tagbanwa", mxCharSet_Script_Extensions_Tagbanwa, gxCharSet_Script_Extensions_Tagbanwa },
	{ "Tai_Le", mxCharSet_Script_Extensions_Tai_Le, gxCharSet_Script_Extensions_Tai_Le },
	{ "Tai_Tham", mxCharSet_Script_Extensions_Tai_Tham, gxCharSet_Script_Extensions_Tai_Tham },
	{ "Tai_Viet", mxCharSet_Script_Extensions_Tai_Viet, gxCharSet_Script_Extensions_Tai_Viet },
	{ "Takr", mxCharSet_Script_Extensions_Takri, gxCharSet_Script_Extensions_Takri },
	{ "Takri", mxCharSet_Script_Extensions_Takri, gxCharSet_Script_Extensions_Takri },
	{ "Tale", mxCharSet_Script_Extensions_Tai_Le, gxCharSet_Script_Extensions_Tai_Le },
	{ "Talu", mxCharSet_Script_Extensions_New_Tai_Lue, gxCharSet_Script_Extensions_New_Tai_Lue },
	{ "Tamil", mxCharSet_Script_Extensions_Tamil, gxCharSet_Script_Extensions_Tamil },
	{ "Taml", mxCharSet_Script_Extensions_Tamil, gxCharSet_Script_Extensions_Tamil },
	{ "Tang", mxCharSet_Script_Extensions_Tangut, gxCharSet_Script_Extensions_Tangut },
	{ "Tangsa", mxCharSet_Script_Extensions_Tangsa, gxCharSet_Script_Extensions_Tangsa },
	{ "Tangut", mxCharSet_Script_Extensions_Tangut, gxCharSet_Script_Extensions_Tangut },
	{ "Tavt", mxCharSet_Script_Extensions_Tai_Viet, gxCharSet_Script_Extensions_Tai_Viet },
	{ "Telu", mxCharSet_Script_Extensions_Telugu, gxCharSet_Script_Extensions_Telugu },
	{ "Telugu", mxCharSet_Script_Extensions_Telugu, gxCharSet_Script_Extensions_Telugu },
	{ "Tfng", mxCharSet_Script_Extensions_Tifinagh, gxCharSet_Script_Extensions_Tifinagh },
	{ "Tglg", mxCharSet_Script_Extensions_Tagalog, gxCharSet_Script_Extensions_Tagalog },
	{ "Thaa", mxCharSet_Script_Extensions_Thaana, gxCharSet_Script_Extensions_Thaana },
	{ "Thaana", mxCharSet_Script_Extensions_Thaana, gxCharSet_Script_Extensions_Thaana },
	{ "Thai", mxCharSet_Script_Extensions_Thai, gxCharSet_Script_Extensions_Thai },
	{ "Tibetan", mxCharSet_Script_Extensions_Tibetan, gxCharSet_Script_Extensions_Tibetan },
	{ "Tibt", mxCharSet_Script_Extensions_Tibetan, gxCharSet_Script_Extensions_Tibetan },
	{ "Tifinagh", mxCharSet_Script_Extensions_Tifinagh, gxCharSet_Script_Extensions_Tifinagh },
	{ "Tirh", mxCharSet_Script_Extensions_Tirhuta, gxCharSet_Script_Extensions_Tirhuta },
	{ "Tirhuta", mxCharSet_Script_Extensions_Tirhuta, gxCharSet_Script_Extensions_Tirhuta },
	{ "Tnsa", mxCharSet_Script_Extensions_Tangsa, gxCharSet_Script_Extensions_Tangsa },
	{ "Toto", mxCharSet_Script_Extensions_Toto, gxCharSet_Script_Extensions_Toto },
	{ "Ugar", mxCharSet_Script_Extensions_Ugaritic, gxCharSet_Script_Extensions_Ugaritic },
	{ "Ugaritic", mxCharSet_Script_Extensions_Ugaritic, gxCharSet_Script_Extensions_Ugaritic },
	{ "Vai", mxCharSet_Script_Extensions_Vai, gxCharSet_Script_Extensions_Vai },
	{ "Vaii", mxCharSet_Script_Extensions_Vai, gxCharSet_Script_Extensions_Vai },
	{ "Vith", mxCharSet_Script_Extensions_Vithkuqi, gxCharSet_Script_Extensions_Vithkuqi },
	{ "Vithkuqi", mxCharSet_Script_Extensions_Vithkuqi, gxCharSet_Script_Extensions_Vithkuqi },
	{ "Wancho", mxCharSet_Script_Extensions_Wancho, gxCharSet_Script_Extensions_Wancho },
	{ "Wara", mxCharSet_Script_Extensions_Warang_Citi, gxCharSet_Script_Extensions_Warang_Citi },
	{ "Warang_Citi", mxCharSet_Script_Extensions_Warang_Citi, gxCharSet_Script_Extensions_Warang_Citi },
	{ "Wcho", mxCharSet_Script_Extensions_Wancho, gxCharSet_Script_Extensions_Wancho },
	{ "Xpeo", mxCharSet_Script_Extensions_Old_Persian, gxCharSet_Script_Extensions_Old_Persian },
	{ "Xsux", mxCharSet_Script_Extensions_Cuneiform, gxCharSet_Script_Extensions_Cuneiform },
	{ "Yezi", mxCharSet_Script_Extensions_Yezidi, gxCharSet_Script_Extensions_Yezidi },
	{ "Yezidi", mxCharSet_Script_Extensions_Yezidi, gxCharSet_Script_Extensions_Yezidi },
	{ "Yi", mxCharSet_Script_Extensions_Yi, gxCharSet_Script_Extensions_Yi },
	{ "Yiii", mxCharSet_Script_Extensions_Yi, gxCharSet_Script_Extensions_Yi },
	{ "Zanabazar_Square", mxCharSet_Script_Extensions_Zanabazar_Square, gxCharSet_Script_Extensions_Zanabazar_Square },
	{ "Zanb", mxCharSet_Script_Extensions_Zanabazar_Square, gxCharSet_Script_Extensions_Zanabazar_Square },
	{ "Zinh", mxCharSet_Script_Extensions_Inherited, gxCharSet_Script_Extensions_Inherited },
	{ "Zyyy", mxCharSet_Script_Extensions_Common, gxCharSet_Script_Extensions_Common },
};

#define mxCharSet_Basic_Emoji 160
static const txInteger ICACHE_RODATA_ATTR gxCharSet_Basic_Emoji[mxCharSet_Basic_Emoji] = {
	0x00231a, 0x00231c, 0x0023e9, 0x0023ed, 0x0023f0, 0x0023f1, 0x0023f3, 0x0023f4, 0x0025fd, 0x0025ff, 0x002614, 0x002616, 0x002648, 0x002654, 0x00267f, 0x002680, 
	0x002693, 0x002694, 0x0026a1, 0x0026a2, 0x0026aa, 0x0026ac, 0x0026bd, 0x0026bf, 0x0026c4, 0x0026c6, 0x0026ce, 0x0026cf, 0x0026d4, 0x0026d5, 0x0026ea, 0x0026eb, 
	0x0026f2, 0x0026f4, 0x0026f5, 0x0026f6, 0x0026fa, 0x0026fb, 0x0026fd, 0x0026fe, 0x002705, 0x002706, 0x00270a, 0x00270c, 0x002728, 0x002729, 0x00274c, 0x00274d, 
	0x00274e, 0x00274f, 0x002753, 0x002756, 0x002757, 0x002758, 0x002795, 0x002798, 0x0027b0, 0x0027b1, 0x0027bf, 0x0027c0, 0x002b1b, 0x002b1d, 0x002b50, 0x002b51, 
	0x002b55, 0x002b56, 0x01f004, 0x01f005, 0x01f0cf, 0x01f0d0, 0x01f18e, 0x01f18f, 0x01f191, 0x01f19b, 0x01f201, 0x01f202, 0x01f21a, 0x01f21b, 0x01f22f, 0x01f230, 
	0x01f232, 0x01f237, 0x01f238, 0x01f23b, 0x01f250, 0x01f252, 0x01f300, 0x01f321, 0x01f32d, 0x01f336, 0x01f337, 0x01f37d, 0x01f37e, 0x01f394, 0x01f3a0, 0x01f3cb, 
	0x01f3cf, 0x01f3d4, 0x01f3e0, 0x01f3f1, 0x01f3f4, 0x01f3f5, 0x01f3f8, 0x01f43f, 0x01f440, 0x01f441, 0x01f442, 0x01f4fd, 0x01f4ff, 0x01f53e, 0x01f54b, 0x01f54f, 
	0x01f550, 0x01f568, 0x01f57a, 0x01f57b, 0x01f595, 0x01f597, 0x01f5a4, 0x01f5a5, 0x01f5fb, 0x01f650, 0x01f680, 0x01f6c6, 0x01f6cc, 0x01f6cd, 0x01f6d0, 0x01f6d3, 
	0x01f6d5, 0x01f6d8, 0x01f6dc, 0x01f6e0, 0x01f6eb, 0x01f6ed, 0x01f6f4, 0x01f6fd, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f90c, 0x01f93b, 0x01f93c, 0x01f946, 
	0x01f947, 0x01fa00, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 
};
#define mxStrings_Basic_Emoji 207
static const txString ICACHE_RODATA_ATTR gxStrings_Basic_Emoji[mxStrings_Basic_Emoji] = {
	"\xF0\x9F\x9B\xB3\xEF\xB8\x8F","\xF0\x9F\x9B\xB0\xEF\xB8\x8F","\xF0\x9F\x9B\xA9\xEF\xB8\x8F","\xF0\x9F\x9B\xA5\xEF\xB8\x8F","\xF0\x9F\x9B\xA4\xEF\xB8\x8F",
	"\xF0\x9F\x9B\xA3\xEF\xB8\x8F","\xF0\x9F\x9B\xA2\xEF\xB8\x8F","\xF0\x9F\x9B\xA1\xEF\xB8\x8F","\xF0\x9F\x9B\xA0\xEF\xB8\x8F","\xF0\x9F\x9B\x8F\xEF\xB8\x8F",
	"\xF0\x9F\x9B\x8E\xEF\xB8\x8F","\xF0\x9F\x9B\x8D\xEF\xB8\x8F","\xF0\x9F\x9B\x8B\xEF\xB8\x8F","\xF0\x9F\x97\xBA\xEF\xB8\x8F","\xF0\x9F\x97\xB3\xEF\xB8\x8F",
	"\xF0\x9F\x97\xAF\xEF\xB8\x8F","\xF0\x9F\x97\xA8\xEF\xB8\x8F","\xF0\x9F\x97\xA3\xEF\xB8\x8F","\xF0\x9F\x97\xA1\xEF\xB8\x8F","\xF0\x9F\x97\x9E\xEF\xB8\x8F",
	"\xF0\x9F\x97\x9D\xEF\xB8\x8F","\xF0\x9F\x97\x9C\xEF\xB8\x8F","\xF0\x9F\x97\x93\xEF\xB8\x8F","\xF0\x9F\x97\x92\xEF\xB8\x8F","\xF0\x9F\x97\x91\xEF\xB8\x8F",
	"\xF0\x9F\x97\x84\xEF\xB8\x8F","\xF0\x9F\x97\x83\xEF\xB8\x8F","\xF0\x9F\x97\x82\xEF\xB8\x8F","\xF0\x9F\x96\xBC\xEF\xB8\x8F","\xF0\x9F\x96\xB2\xEF\xB8\x8F",
	"\xF0\x9F\x96\xB1\xEF\xB8\x8F","\xF0\x9F\x96\xA8\xEF\xB8\x8F","\xF0\x9F\x96\xA5\xEF\xB8\x8F","\xF0\x9F\x96\x90\xEF\xB8\x8F","\xF0\x9F\x96\x8D\xEF\xB8\x8F",
	"\xF0\x9F\x96\x8C\xEF\xB8\x8F","\xF0\x9F\x96\x8B\xEF\xB8\x8F","\xF0\x9F\x96\x8A\xEF\xB8\x8F","\xF0\x9F\x96\x87\xEF\xB8\x8F","\xF0\x9F\x95\xB9\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB8\xEF\xB8\x8F","\xF0\x9F\x95\xB7\xEF\xB8\x8F","\xF0\x9F\x95\xB6\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xEF\xB8\x8F","\xF0\x9F\x95\xB4\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB3\xEF\xB8\x8F","\xF0\x9F\x95\xB0\xEF\xB8\x8F","\xF0\x9F\x95\xAF\xEF\xB8\x8F","\xF0\x9F\x95\x8A\xEF\xB8\x8F","\xF0\x9F\x95\x89\xEF\xB8\x8F",
	"\xF0\x9F\x93\xBD\xEF\xB8\x8F","\xF0\x9F\x91\x81\xEF\xB8\x8F","\xF0\x9F\x90\xBF\xEF\xB8\x8F","\xF0\x9F\x8F\xB7\xEF\xB8\x8F","\xF0\x9F\x8F\xB5\xEF\xB8\x8F",
	"\xF0\x9F\x8F\xB3\xEF\xB8\x8F","\xF0\x9F\x8F\x9F\xEF\xB8\x8F","\xF0\x9F\x8F\x9E\xEF\xB8\x8F","\xF0\x9F\x8F\x9D\xEF\xB8\x8F","\xF0\x9F\x8F\x9C\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x9B\xEF\xB8\x8F","\xF0\x9F\x8F\x9A\xEF\xB8\x8F","\xF0\x9F\x8F\x99\xEF\xB8\x8F","\xF0\x9F\x8F\x98\xEF\xB8\x8F","\xF0\x9F\x8F\x97\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x96\xEF\xB8\x8F","\xF0\x9F\x8F\x95\xEF\xB8\x8F","\xF0\x9F\x8F\x94\xEF\xB8\x8F","\xF0\x9F\x8F\x8E\xEF\xB8\x8F","\xF0\x9F\x8F\x8D\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xEF\xB8\x8F","\xF0\x9F\x8E\x9F\xEF\xB8\x8F","\xF0\x9F\x8E\x9E\xEF\xB8\x8F","\xF0\x9F\x8E\x9B\xEF\xB8\x8F",
	"\xF0\x9F\x8E\x9A\xEF\xB8\x8F","\xF0\x9F\x8E\x99\xEF\xB8\x8F","\xF0\x9F\x8E\x97\xEF\xB8\x8F","\xF0\x9F\x8E\x96\xEF\xB8\x8F","\xF0\x9F\x8D\xBD\xEF\xB8\x8F",
	"\xF0\x9F\x8C\xB6\xEF\xB8\x8F","\xF0\x9F\x8C\xAC\xEF\xB8\x8F","\xF0\x9F\x8C\xAB\xEF\xB8\x8F","\xF0\x9F\x8C\xAA\xEF\xB8\x8F","\xF0\x9F\x8C\xA9\xEF\xB8\x8F",
	"\xF0\x9F\x8C\xA8\xEF\xB8\x8F","\xF0\x9F\x8C\xA7\xEF\xB8\x8F","\xF0\x9F\x8C\xA6\xEF\xB8\x8F","\xF0\x9F\x8C\xA5\xEF\xB8\x8F","\xF0\x9F\x8C\xA4\xEF\xB8\x8F",
	"\xF0\x9F\x8C\xA1\xEF\xB8\x8F","\xF0\x9F\x88\xB7\xEF\xB8\x8F","\xF0\x9F\x88\x82\xEF\xB8\x8F","\xF0\x9F\x85\xBF\xEF\xB8\x8F","\xF0\x9F\x85\xBE\xEF\xB8\x8F",
	"\xF0\x9F\x85\xB1\xEF\xB8\x8F","\xF0\x9F\x85\xB0\xEF\xB8\x8F","\xE3\x8A\x99\xEF\xB8\x8F","\xE3\x8A\x97\xEF\xB8\x8F","\xE3\x80\xBD\xEF\xB8\x8F",
	"\xE3\x80\xB0\xEF\xB8\x8F","\xE2\xAC\x87\xEF\xB8\x8F","\xE2\xAC\x86\xEF\xB8\x8F","\xE2\xAC\x85\xEF\xB8\x8F","\xE2\xA4\xB5\xEF\xB8\x8F",
	"\xE2\xA4\xB4\xEF\xB8\x8F","\xE2\x9E\xA1\xEF\xB8\x8F","\xE2\x9D\xA4\xEF\xB8\x8F","\xE2\x9D\xA3\xEF\xB8\x8F","\xE2\x9D\x87\xEF\xB8\x8F",
	"\xE2\x9D\x84\xEF\xB8\x8F","\xE2\x9C\xB4\xEF\xB8\x8F","\xE2\x9C\xB3\xEF\xB8\x8F","\xE2\x9C\xA1\xEF\xB8\x8F","\xE2\x9C\x9D\xEF\xB8\x8F",
	"\xE2\x9C\x96\xEF\xB8\x8F","\xE2\x9C\x94\xEF\xB8\x8F","\xE2\x9C\x92\xEF\xB8\x8F","\xE2\x9C\x8F\xEF\xB8\x8F","\xE2\x9C\x8D\xEF\xB8\x8F",
	"\xE2\x9C\x8C\xEF\xB8\x8F","\xE2\x9C\x89\xEF\xB8\x8F","\xE2\x9C\x88\xEF\xB8\x8F","\xE2\x9C\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xEF\xB8\x8F",
	"\xE2\x9B\xB8\xEF\xB8\x8F","\xE2\x9B\xB7\xEF\xB8\x8F","\xE2\x9B\xB4\xEF\xB8\x8F","\xE2\x9B\xB1\xEF\xB8\x8F","\xE2\x9B\xB0\xEF\xB8\x8F",
	"\xE2\x9B\xA9\xEF\xB8\x8F","\xE2\x9B\x93\xEF\xB8\x8F","\xE2\x9B\x91\xEF\xB8\x8F","\xE2\x9B\x8F\xEF\xB8\x8F","\xE2\x9B\x88\xEF\xB8\x8F",
	"\xE2\x9A\xB1\xEF\xB8\x8F","\xE2\x9A\xB0\xEF\xB8\x8F","\xE2\x9A\xA7\xEF\xB8\x8F","\xE2\x9A\xA0\xEF\xB8\x8F","\xE2\x9A\x9C\xEF\xB8\x8F",
	"\xE2\x9A\x9B\xEF\xB8\x8F","\xE2\x9A\x99\xEF\xB8\x8F","\xE2\x9A\x97\xEF\xB8\x8F","\xE2\x9A\x96\xEF\xB8\x8F","\xE2\x9A\x95\xEF\xB8\x8F",
	"\xE2\x9A\x94\xEF\xB8\x8F","\xE2\x9A\x92\xEF\xB8\x8F","\xE2\x99\xBE\xEF\xB8\x8F","\xE2\x99\xBB\xEF\xB8\x8F","\xE2\x99\xA8\xEF\xB8\x8F",
	"\xE2\x99\xA6\xEF\xB8\x8F","\xE2\x99\xA5\xEF\xB8\x8F","\xE2\x99\xA3\xEF\xB8\x8F","\xE2\x99\xA0\xEF\xB8\x8F","\xE2\x99\x9F\xEF\xB8\x8F",
	"\xE2\x99\x82\xEF\xB8\x8F","\xE2\x99\x80\xEF\xB8\x8F","\xE2\x98\xBA\xEF\xB8\x8F","\xE2\x98\xB9\xEF\xB8\x8F","\xE2\x98\xB8\xEF\xB8\x8F",
	"\xE2\x98\xAF\xEF\xB8\x8F","\xE2\x98\xAE\xEF\xB8\x8F","\xE2\x98\xAA\xEF\xB8\x8F","\xE2\x98\xA6\xEF\xB8\x8F","\xE2\x98\xA3\xEF\xB8\x8F",
	"\xE2\x98\xA2\xEF\xB8\x8F","\xE2\x98\xA0\xEF\xB8\x8F","\xE2\x98\x9D\xEF\xB8\x8F","\xE2\x98\x98\xEF\xB8\x8F","\xE2\x98\x91\xEF\xB8\x8F",
	"\xE2\x98\x8E\xEF\xB8\x8F","\xE2\x98\x84\xEF\xB8\x8F","\xE2\x98\x83\xEF\xB8\x8F","\xE2\x98\x82\xEF\xB8\x8F","\xE2\x98\x81\xEF\xB8\x8F",
	"\xE2\x98\x80\xEF\xB8\x8F","\xE2\x97\xBC\xEF\xB8\x8F","\xE2\x97\xBB\xEF\xB8\x8F","\xE2\x97\x80\xEF\xB8\x8F","\xE2\x96\xB6\xEF\xB8\x8F",
	"\xE2\x96\xAB\xEF\xB8\x8F","\xE2\x96\xAA\xEF\xB8\x8F","\xE2\x93\x82\xEF\xB8\x8F","\xE2\x8F\xBA\xEF\xB8\x8F","\xE2\x8F\xB9\xEF\xB8\x8F",
	"\xE2\x8F\xB8\xEF\xB8\x8F","\xE2\x8F\xB2\xEF\xB8\x8F","\xE2\x8F\xB1\xEF\xB8\x8F","\xE2\x8F\xAF\xEF\xB8\x8F","\xE2\x8F\xAE\xEF\xB8\x8F",
	"\xE2\x8F\xAD\xEF\xB8\x8F","\xE2\x8F\x8F\xEF\xB8\x8F","\xE2\x8C\xA8\xEF\xB8\x8F","\xE2\x86\xAA\xEF\xB8\x8F","\xE2\x86\xA9\xEF\xB8\x8F",
	"\xE2\x86\x99\xEF\xB8\x8F","\xE2\x86\x98\xEF\xB8\x8F","\xE2\x86\x97\xEF\xB8\x8F","\xE2\x86\x96\xEF\xB8\x8F","\xE2\x86\x95\xEF\xB8\x8F",
	"\xE2\x86\x94\xEF\xB8\x8F","\xE2\x84\xB9\xEF\xB8\x8F","\xE2\x84\xA2\xEF\xB8\x8F","\xE2\x81\x89\xEF\xB8\x8F","\xE2\x80\xBC\xEF\xB8\x8F","\xC2\xAE\xEF\xB8\x8F",
	"\xC2\xA9\xEF\xB8\x8F",
};
#define mxCharSet_Emoji_Keycap_Sequence 0
#define gxCharSet_Emoji_Keycap_Sequence C_NULL
#define mxStrings_Emoji_Keycap_Sequence 12
static const txString ICACHE_RODATA_ATTR gxStrings_Emoji_Keycap_Sequence[mxStrings_Emoji_Keycap_Sequence] = {
	"9\xEF\xB8\x8F\xE2\x83\xA3","8\xEF\xB8\x8F\xE2\x83\xA3","7\xEF\xB8\x8F\xE2\x83\xA3","6\xEF\xB8\x8F\xE2\x83\xA3","5\xEF\xB8\x8F\xE2\x83\xA3",
	"4\xEF\xB8\x8F\xE2\x83\xA3","3\xEF\xB8\x8F\xE2\x83\xA3","2\xEF\xB8\x8F\xE2\x83\xA3","1\xEF\xB8\x8F\xE2\x83\xA3","0\xEF\xB8\x8F\xE2\x83\xA3",
	"*\xEF\xB8\x8F\xE2\x83\xA3","#\xEF\xB8\x8F\xE2\x83\xA3",
};
#define mxCharSet_RGI_Emoji 160
static const txInteger ICACHE_RODATA_ATTR gxCharSet_RGI_Emoji[mxCharSet_RGI_Emoji] = {
	0x00231a, 0x00231c, 0x0023e9, 0x0023ed, 0x0023f0, 0x0023f1, 0x0023f3, 0x0023f4, 0x0025fd, 0x0025ff, 0x002614, 0x002616, 0x002648, 0x002654, 0x00267f, 0x002680, 
	0x002693, 0x002694, 0x0026a1, 0x0026a2, 0x0026aa, 0x0026ac, 0x0026bd, 0x0026bf, 0x0026c4, 0x0026c6, 0x0026ce, 0x0026cf, 0x0026d4, 0x0026d5, 0x0026ea, 0x0026eb, 
	0x0026f2, 0x0026f4, 0x0026f5, 0x0026f6, 0x0026fa, 0x0026fb, 0x0026fd, 0x0026fe, 0x002705, 0x002706, 0x00270a, 0x00270c, 0x002728, 0x002729, 0x00274c, 0x00274d, 
	0x00274e, 0x00274f, 0x002753, 0x002756, 0x002757, 0x002758, 0x002795, 0x002798, 0x0027b0, 0x0027b1, 0x0027bf, 0x0027c0, 0x002b1b, 0x002b1d, 0x002b50, 0x002b51, 
	0x002b55, 0x002b56, 0x01f004, 0x01f005, 0x01f0cf, 0x01f0d0, 0x01f18e, 0x01f18f, 0x01f191, 0x01f19b, 0x01f201, 0x01f202, 0x01f21a, 0x01f21b, 0x01f22f, 0x01f230, 
	0x01f232, 0x01f237, 0x01f238, 0x01f23b, 0x01f250, 0x01f252, 0x01f300, 0x01f321, 0x01f32d, 0x01f336, 0x01f337, 0x01f37d, 0x01f37e, 0x01f394, 0x01f3a0, 0x01f3cb, 
	0x01f3cf, 0x01f3d4, 0x01f3e0, 0x01f3f1, 0x01f3f4, 0x01f3f5, 0x01f3f8, 0x01f43f, 0x01f440, 0x01f441, 0x01f442, 0x01f4fd, 0x01f4ff, 0x01f53e, 0x01f54b, 0x01f54f, 
	0x01f550, 0x01f568, 0x01f57a, 0x01f57b, 0x01f595, 0x01f597, 0x01f5a4, 0x01f5a5, 0x01f5fb, 0x01f650, 0x01f680, 0x01f6c6, 0x01f6cc, 0x01f6cd, 0x01f6d0, 0x01f6d3, 
	0x01f6d5, 0x01f6d8, 0x01f6dc, 0x01f6e0, 0x01f6eb, 0x01f6ed, 0x01f6f4, 0x01f6fd, 0x01f7e0, 0x01f7ec, 0x01f7f0, 0x01f7f1, 0x01f90c, 0x01f93b, 0x01f93c, 0x01f946, 
	0x01f947, 0x01fa00, 0x01fa70, 0x01fa7d, 0x01fa80, 0x01fa89, 0x01fa90, 0x01fabe, 0x01fabf, 0x01fac6, 0x01face, 0x01fadc, 0x01fae0, 0x01fae9, 0x01faf0, 0x01faf9, 
};
#define mxStrings_RGI_Emoji 2603
static const txString ICACHE_RODATA_ATTR gxStrings_RGI_Emoji[mxStrings_RGI_Emoji] = {
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x92\xE2\x80\x8D\xF0\x9F\xA7\x92",
	"\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB7\xF3\xA0\x81\xAC\xF3\xA0\x81\xB3\xF3\xA0\x81\xBF",
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF",
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xA5\xF3\xA0\x81\xAE\xF3\xA0\x81\xA7\xF3\xA0\x81\xBF",
	"\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xAF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x92\xE2\x80\x8D\xF0\x9F\xA7\x92",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x92","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\x81\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x97\xA8\xEF\xB8\x8F","\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9A\xA7\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9B\xB9\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x84",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x84",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x84",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x84",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x84",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x82\xE2\x80\x8D\xE2\x86\x95\xEF\xB8\x8F","\xF0\x9F\x99\x82\xE2\x80\x8D\xE2\x86\x94\xEF\xB8\x8F",
	"\xF0\x9F\x98\xB6\xE2\x80\x8D\xF0\x9F\x8C\xAB\xEF\xB8\x8F","\xF0\x9F\x92\x87\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x92\x86\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x92\x82\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x92\x81\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xAF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x90\xBB\xE2\x80\x8D\xE2\x9D\x84\xEF\xB8\x8F",
	"\xF0\x9F\x8F\xB4\xE2\x80\x8D\xE2\x98\xA0\xEF\xB8\x8F","\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x8C\x88",
	"\xF0\x9F\x8F\x8A\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x9E\xA1\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA9\xB9",
	"\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x94\xA5","\xE2\x9B\x93\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\xA5","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA7\x92",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x98\xB5\xE2\x80\x8D\xF0\x9F\x92\xAB","\xF0\x9F\x98\xAE\xE2\x80\x8D\xF0\x9F\x92\xA8",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x90\xA6\xE2\x80\x8D\xF0\x9F\x94\xA5","\xF0\x9F\x90\xA6\xE2\x80\x8D\xE2\xAC\x9B",
	"\xF0\x9F\x90\x95\xE2\x80\x8D\xF0\x9F\xA6\xBA","\xF0\x9F\x90\x88\xE2\x80\x8D\xE2\xAC\x9B","\xF0\x9F\x8D\x8B\xE2\x80\x8D\xF0\x9F\x9F\xA9",
	"\xF0\x9F\x8D\x84\xE2\x80\x8D\xF0\x9F\x9F\xAB","9\xEF\xB8\x8F\xE2\x83\xA3","8\xEF\xB8\x8F\xE2\x83\xA3","7\xEF\xB8\x8F\xE2\x83\xA3","6\xEF\xB8\x8F\xE2\x83\xA3",
	"5\xEF\xB8\x8F\xE2\x83\xA3","4\xEF\xB8\x8F\xE2\x83\xA3","3\xEF\xB8\x8F\xE2\x83\xA3","2\xEF\xB8\x8F\xE2\x83\xA3","1\xEF\xB8\x8F\xE2\x83\xA3",
	"0\xEF\xB8\x8F\xE2\x83\xA3","*\xEF\xB8\x8F\xE2\x83\xA3","#\xEF\xB8\x8F\xE2\x83\xA3","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBC","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x9B\xB3\xEF\xB8\x8F",
	"\xF0\x9F\x9B\xB0\xEF\xB8\x8F","\xF0\x9F\x9B\xA9\xEF\xB8\x8F","\xF0\x9F\x9B\xA5\xEF\xB8\x8F","\xF0\x9F\x9B\xA4\xEF\xB8\x8F","\xF0\x9F\x9B\xA3\xEF\xB8\x8F",
	"\xF0\x9F\x9B\xA2\xEF\xB8\x8F","\xF0\x9F\x9B\xA1\xEF\xB8\x8F","\xF0\x9F\x9B\xA0\xEF\xB8\x8F","\xF0\x9F\x9B\x8F\xEF\xB8\x8F","\xF0\x9F\x9B\x8E\xEF\xB8\x8F",
	"\xF0\x9F\x9B\x8D\xEF\xB8\x8F","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x9B\x8B\xEF\xB8\x8F","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBE","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBD","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBC","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x97\xBA\xEF\xB8\x8F","\xF0\x9F\x97\xB3\xEF\xB8\x8F","\xF0\x9F\x97\xAF\xEF\xB8\x8F","\xF0\x9F\x97\xA8\xEF\xB8\x8F","\xF0\x9F\x97\xA3\xEF\xB8\x8F",
	"\xF0\x9F\x97\xA1\xEF\xB8\x8F","\xF0\x9F\x97\x9E\xEF\xB8\x8F","\xF0\x9F\x97\x9D\xEF\xB8\x8F","\xF0\x9F\x97\x9C\xEF\xB8\x8F","\xF0\x9F\x97\x93\xEF\xB8\x8F",
	"\xF0\x9F\x97\x92\xEF\xB8\x8F","\xF0\x9F\x97\x91\xEF\xB8\x8F","\xF0\x9F\x97\x84\xEF\xB8\x8F","\xF0\x9F\x97\x83\xEF\xB8\x8F","\xF0\x9F\x97\x82\xEF\xB8\x8F",
	"\xF0\x9F\x96\xBC\xEF\xB8\x8F","\xF0\x9F\x96\xB2\xEF\xB8\x8F","\xF0\x9F\x96\xB1\xEF\xB8\x8F","\xF0\x9F\x96\xA8\xEF\xB8\x8F","\xF0\x9F\x96\xA5\xEF\xB8\x8F",
	"\xF0\x9F\x96\x96\xF0\x9F\x8F\xBF","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBE","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBD","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x96\x96\xF0\x9F\x8F\xBB","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBF","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBE","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x96\x95\xF0\x9F\x8F\xBC","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBB","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBF","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x96\x90\xF0\x9F\x8F\xBD","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBC","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBB","\xF0\x9F\x96\x90\xEF\xB8\x8F",
	"\xF0\x9F\x96\x8D\xEF\xB8\x8F","\xF0\x9F\x96\x8C\xEF\xB8\x8F","\xF0\x9F\x96\x8B\xEF\xB8\x8F","\xF0\x9F\x96\x8A\xEF\xB8\x8F","\xF0\x9F\x96\x87\xEF\xB8\x8F",
	"\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBF","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBE","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBB","\xF0\x9F\x95\xB9\xEF\xB8\x8F","\xF0\x9F\x95\xB8\xEF\xB8\x8F","\xF0\x9F\x95\xB7\xEF\xB8\x8F","\xF0\x9F\x95\xB6\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\x95\xB5\xEF\xB8\x8F","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\x95\xB4\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB3\xEF\xB8\x8F","\xF0\x9F\x95\xB0\xEF\xB8\x8F","\xF0\x9F\x95\xAF\xEF\xB8\x8F","\xF0\x9F\x95\x8A\xEF\xB8\x8F","\xF0\x9F\x95\x89\xEF\xB8\x8F",
	"\xF0\x9F\x93\xBD\xEF\xB8\x8F","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBF","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBE","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBC","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x92\x91\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x92\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x92\x83\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x90\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x90\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x89\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x88\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x88\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x87\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x86\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x83\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x82\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x82\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x81\xEF\xB8\x8F","\xF0\x9F\x90\xBF\xEF\xB8\x8F","\xF0\x9F\x8F\xB7\xEF\xB8\x8F","\xF0\x9F\x8F\xB5\xEF\xB8\x8F",
	"\xF0\x9F\x8F\xB3\xEF\xB8\x8F","\xF0\x9F\x8F\x9F\xEF\xB8\x8F","\xF0\x9F\x8F\x9E\xEF\xB8\x8F","\xF0\x9F\x8F\x9D\xEF\xB8\x8F","\xF0\x9F\x8F\x9C\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x9B\xEF\xB8\x8F","\xF0\x9F\x8F\x9A\xEF\xB8\x8F","\xF0\x9F\x8F\x99\xEF\xB8\x8F","\xF0\x9F\x8F\x98\xEF\xB8\x8F","\xF0\x9F\x8F\x97\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x96\xEF\xB8\x8F","\xF0\x9F\x8F\x95\xEF\xB8\x8F","\xF0\x9F\x8F\x94\xEF\xB8\x8F","\xF0\x9F\x8F\x8E\xEF\xB8\x8F","\xF0\x9F\x8F\x8D\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x8C\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x8B\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBB","\xF0\x9F\x8E\x9F\xEF\xB8\x8F","\xF0\x9F\x8E\x9E\xEF\xB8\x8F","\xF0\x9F\x8E\x9B\xEF\xB8\x8F","\xF0\x9F\x8E\x9A\xEF\xB8\x8F",
	"\xF0\x9F\x8E\x99\xEF\xB8\x8F","\xF0\x9F\x8E\x97\xEF\xB8\x8F","\xF0\x9F\x8E\x96\xEF\xB8\x8F","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBE","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBC","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x8D\xBD\xEF\xB8\x8F","\xF0\x9F\x8C\xB6\xEF\xB8\x8F","\xF0\x9F\x8C\xAC\xEF\xB8\x8F","\xF0\x9F\x8C\xAB\xEF\xB8\x8F","\xF0\x9F\x8C\xAA\xEF\xB8\x8F",
	"\xF0\x9F\x8C\xA9\xEF\xB8\x8F","\xF0\x9F\x8C\xA8\xEF\xB8\x8F","\xF0\x9F\x8C\xA7\xEF\xB8\x8F","\xF0\x9F\x8C\xA6\xEF\xB8\x8F","\xF0\x9F\x8C\xA5\xEF\xB8\x8F",
	"\xF0\x9F\x8C\xA4\xEF\xB8\x8F","\xF0\x9F\x8C\xA1\xEF\xB8\x8F","\xF0\x9F\x88\xB7\xEF\xB8\x8F","\xF0\x9F\x88\x82\xEF\xB8\x8F","\xF0\x9F\x87\xBF\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xBF\xF0\x9F\x87\xB2","\xF0\x9F\x87\xBF\xF0\x9F\x87\xA6","\xF0\x9F\x87\xBE\xF0\x9F\x87\xB9","\xF0\x9F\x87\xBE\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xBD\xF0\x9F\x87\xB0","\xF0\x9F\x87\xBC\xF0\x9F\x87\xB8","\xF0\x9F\x87\xBC\xF0\x9F\x87\xAB","\xF0\x9F\x87\xBB\xF0\x9F\x87\xBA",
	"\xF0\x9F\x87\xBB\xF0\x9F\x87\xB3","\xF0\x9F\x87\xBB\xF0\x9F\x87\xAE","\xF0\x9F\x87\xBB\xF0\x9F\x87\xAC","\xF0\x9F\x87\xBB\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xBB\xF0\x9F\x87\xA8","\xF0\x9F\x87\xBB\xF0\x9F\x87\xA6","\xF0\x9F\x87\xBA\xF0\x9F\x87\xBF","\xF0\x9F\x87\xBA\xF0\x9F\x87\xBE",
	"\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8","\xF0\x9F\x87\xBA\xF0\x9F\x87\xB3","\xF0\x9F\x87\xBA\xF0\x9F\x87\xB2","\xF0\x9F\x87\xBA\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xBA\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB9\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB9\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB9\xF0\x9F\x87\xBB",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAF",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB9\xF0\x9F\x87\xA9",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB9\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBE",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xBD","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB8",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAF","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAE",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA9",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA7","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB7\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xB7\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB7\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB7\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB7\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xB6\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB5\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB5\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB5\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB4\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB5",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAE","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB3\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB3\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBD","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB8",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB6","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB5","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB0",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xA9","\xF0\x9F\x87\xB2\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB2\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB1\xF0\x9F\x87\xBE",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB1\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB8",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB1\xF0\x9F\x87\xAE","\xF0\x9F\x87\xB1\xF0\x9F\x87\xA8",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xA7","\xF0\x9F\x87\xB1\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB0\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB0\xF0\x9F\x87\xBE",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB5","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAE","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAF\xF0\x9F\x87\xB5","\xF0\x9F\x87\xAF\xF0\x9F\x87\xB4","\xF0\x9F\x87\xAF\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xAF\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB8","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xAE\xF0\x9F\x87\xB6","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB4","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB3","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xAE\xF0\x9F\x87\xB1","\xF0\x9F\x87\xAE\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAE\xF0\x9F\x87\xA9","\xF0\x9F\x87\xAE\xF0\x9F\x87\xA8",
	"\xF0\x9F\x87\xAD\xF0\x9F\x87\xBA","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xAD\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB0","\xF0\x9F\x87\xAC\xF0\x9F\x87\xBE","\xF0\x9F\x87\xAC\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xBA","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB8","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xB6","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB5","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB3","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xB1","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAE","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAD","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xAB","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAC\xF0\x9F\x87\xA9","\xF0\x9F\x87\xAC\xF0\x9F\x87\xA7",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xA6","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB4","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xAB\xF0\x9F\x87\xB0","\xF0\x9F\x87\xAB\xF0\x9F\x87\xAF","\xF0\x9F\x87\xAB\xF0\x9F\x87\xAE","\xF0\x9F\x87\xAA\xF0\x9F\x87\xBA",
	"\xF0\x9F\x87\xAA\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAA\xF0\x9F\x87\xB8","\xF0\x9F\x87\xAA\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAA\xF0\x9F\x87\xAD",
	"\xF0\x9F\x87\xAA\xF0\x9F\x87\xAC","\xF0\x9F\x87\xAA\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAA\xF0\x9F\x87\xA8","\xF0\x9F\x87\xAA\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xA9\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA9\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA9\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA9\xF0\x9F\x87\xB0",
	"\xF0\x9F\x87\xA9\xF0\x9F\x87\xAF","\xF0\x9F\x87\xA9\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA9\xF0\x9F\x87\xAA","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBF",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xBE","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBD","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBC","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBB",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xBA","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB7","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB5","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB1","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB0",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xAE","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAD","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAB",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xA9","\xF0\x9F\x87\xA8\xF0\x9F\x87\xA8","\xF0\x9F\x87\xA8\xF0\x9F\x87\xA6","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBF",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xBE","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBC","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBB","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xB8","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB7","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB6","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xB3","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB1","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAF",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xAE","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAD","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAB",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xAA","\xF0\x9F\x87\xA7\xF0\x9F\x87\xA9","\xF0\x9F\x87\xA7\xF0\x9F\x87\xA7","\xF0\x9F\x87\xA7\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBD","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBC","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBA",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xB9","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB8","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB7","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB6",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB1","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAE",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAB","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAA","\xF0\x9F\x87\xA6\xF0\x9F\x87\xA9",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xA8","\xF0\x9F\x85\xBF\xEF\xB8\x8F","\xF0\x9F\x85\xBE\xEF\xB8\x8F","\xF0\x9F\x85\xB1\xEF\xB8\x8F","\xF0\x9F\x85\xB0\xEF\xB8\x8F",
	"\xE3\x8A\x99\xEF\xB8\x8F","\xE3\x8A\x97\xEF\xB8\x8F","\xE3\x80\xBD\xEF\xB8\x8F","\xE3\x80\xB0\xEF\xB8\x8F","\xE2\xAC\x87\xEF\xB8\x8F",
	"\xE2\xAC\x86\xEF\xB8\x8F","\xE2\xAC\x85\xEF\xB8\x8F","\xE2\xA4\xB5\xEF\xB8\x8F","\xE2\xA4\xB4\xEF\xB8\x8F","\xE2\x9E\xA1\xEF\xB8\x8F",
	"\xE2\x9D\xA4\xEF\xB8\x8F","\xE2\x9D\xA3\xEF\xB8\x8F","\xE2\x9D\x87\xEF\xB8\x8F","\xE2\x9D\x84\xEF\xB8\x8F","\xE2\x9C\xB4\xEF\xB8\x8F",
	"\xE2\x9C\xB3\xEF\xB8\x8F","\xE2\x9C\xA1\xEF\xB8\x8F","\xE2\x9C\x9D\xEF\xB8\x8F","\xE2\x9C\x96\xEF\xB8\x8F","\xE2\x9C\x94\xEF\xB8\x8F",
	"\xE2\x9C\x92\xEF\xB8\x8F","\xE2\x9C\x8F\xEF\xB8\x8F","\xE2\x9C\x8D\xF0\x9F\x8F\xBF","\xE2\x9C\x8D\xF0\x9F\x8F\xBE","\xE2\x9C\x8D\xF0\x9F\x8F\xBD",
	"\xE2\x9C\x8D\xF0\x9F\x8F\xBC","\xE2\x9C\x8D\xF0\x9F\x8F\xBB","\xE2\x9C\x8D\xEF\xB8\x8F","\xE2\x9C\x8C\xF0\x9F\x8F\xBF","\xE2\x9C\x8C\xF0\x9F\x8F\xBE",
	"\xE2\x9C\x8C\xF0\x9F\x8F\xBD","\xE2\x9C\x8C\xF0\x9F\x8F\xBC","\xE2\x9C\x8C\xF0\x9F\x8F\xBB","\xE2\x9C\x8C\xEF\xB8\x8F","\xE2\x9C\x8B\xF0\x9F\x8F\xBF",
	"\xE2\x9C\x8B\xF0\x9F\x8F\xBE","\xE2\x9C\x8B\xF0\x9F\x8F\xBD","\xE2\x9C\x8B\xF0\x9F\x8F\xBC","\xE2\x9C\x8B\xF0\x9F\x8F\xBB","\xE2\x9C\x8A\xF0\x9F\x8F\xBF",
	"\xE2\x9C\x8A\xF0\x9F\x8F\xBE","\xE2\x9C\x8A\xF0\x9F\x8F\xBD","\xE2\x9C\x8A\xF0\x9F\x8F\xBC","\xE2\x9C\x8A\xF0\x9F\x8F\xBB","\xE2\x9C\x89\xEF\xB8\x8F",
	"\xE2\x9C\x88\xEF\xB8\x8F","\xE2\x9C\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBF","\xE2\x9B\xB9\xF0\x9F\x8F\xBE","\xE2\x9B\xB9\xF0\x9F\x8F\xBD",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBC","\xE2\x9B\xB9\xF0\x9F\x8F\xBB","\xE2\x9B\xB9\xEF\xB8\x8F","\xE2\x9B\xB8\xEF\xB8\x8F","\xE2\x9B\xB7\xEF\xB8\x8F",
	"\xE2\x9B\xB4\xEF\xB8\x8F","\xE2\x9B\xB1\xEF\xB8\x8F","\xE2\x9B\xB0\xEF\xB8\x8F","\xE2\x9B\xA9\xEF\xB8\x8F","\xE2\x9B\x93\xEF\xB8\x8F",
	"\xE2\x9B\x91\xEF\xB8\x8F","\xE2\x9B\x8F\xEF\xB8\x8F","\xE2\x9B\x88\xEF\xB8\x8F","\xE2\x9A\xB1\xEF\xB8\x8F","\xE2\x9A\xB0\xEF\xB8\x8F",
	"\xE2\x9A\xA7\xEF\xB8\x8F","\xE2\x9A\xA0\xEF\xB8\x8F","\xE2\x9A\x9C\xEF\xB8\x8F","\xE2\x9A\x9B\xEF\xB8\x8F","\xE2\x9A\x99\xEF\xB8\x8F",
	"\xE2\x9A\x97\xEF\xB8\x8F","\xE2\x9A\x96\xEF\xB8\x8F","\xE2\x9A\x95\xEF\xB8\x8F","\xE2\x9A\x94\xEF\xB8\x8F","\xE2\x9A\x92\xEF\xB8\x8F",
	"\xE2\x99\xBE\xEF\xB8\x8F","\xE2\x99\xBB\xEF\xB8\x8F","\xE2\x99\xA8\xEF\xB8\x8F","\xE2\x99\xA6\xEF\xB8\x8F","\xE2\x99\xA5\xEF\xB8\x8F",
	"\xE2\x99\xA3\xEF\xB8\x8F","\xE2\x99\xA0\xEF\xB8\x8F","\xE2\x99\x9F\xEF\xB8\x8F","\xE2\x99\x82\xEF\xB8\x8F","\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x98\xBA\xEF\xB8\x8F","\xE2\x98\xB9\xEF\xB8\x8F","\xE2\x98\xB8\xEF\xB8\x8F","\xE2\x98\xAF\xEF\xB8\x8F","\xE2\x98\xAE\xEF\xB8\x8F",
	"\xE2\x98\xAA\xEF\xB8\x8F","\xE2\x98\xA6\xEF\xB8\x8F","\xE2\x98\xA3\xEF\xB8\x8F","\xE2\x98\xA2\xEF\xB8\x8F","\xE2\x98\xA0\xEF\xB8\x8F",
	"\xE2\x98\x9D\xF0\x9F\x8F\xBF","\xE2\x98\x9D\xF0\x9F\x8F\xBE","\xE2\x98\x9D\xF0\x9F\x8F\xBD","\xE2\x98\x9D\xF0\x9F\x8F\xBC","\xE2\x98\x9D\xF0\x9F\x8F\xBB",
	"\xE2\x98\x9D\xEF\xB8\x8F","\xE2\x98\x98\xEF\xB8\x8F","\xE2\x98\x91\xEF\xB8\x8F","\xE2\x98\x8E\xEF\xB8\x8F","\xE2\x98\x84\xEF\xB8\x8F",
	"\xE2\x98\x83\xEF\xB8\x8F","\xE2\x98\x82\xEF\xB8\x8F","\xE2\x98\x81\xEF\xB8\x8F","\xE2\x98\x80\xEF\xB8\x8F","\xE2\x97\xBC\xEF\xB8\x8F",
	"\xE2\x97\xBB\xEF\xB8\x8F","\xE2\x97\x80\xEF\xB8\x8F","\xE2\x96\xB6\xEF\xB8\x8F","\xE2\x96\xAB\xEF\xB8\x8F","\xE2\x96\xAA\xEF\xB8\x8F",
	"\xE2\x93\x82\xEF\xB8\x8F","\xE2\x8F\xBA\xEF\xB8\x8F","\xE2\x8F\xB9\xEF\xB8\x8F","\xE2\x8F\xB8\xEF\xB8\x8F","\xE2\x8F\xB2\xEF\xB8\x8F",
	"\xE2\x8F\xB1\xEF\xB8\x8F","\xE2\x8F\xAF\xEF\xB8\x8F","\xE2\x8F\xAE\xEF\xB8\x8F","\xE2\x8F\xAD\xEF\xB8\x8F","\xE2\x8F\x8F\xEF\xB8\x8F",
	"\xE2\x8C\xA8\xEF\xB8\x8F","\xE2\x86\xAA\xEF\xB8\x8F","\xE2\x86\xA9\xEF\xB8\x8F","\xE2\x86\x99\xEF\xB8\x8F","\xE2\x86\x98\xEF\xB8\x8F",
	"\xE2\x86\x97\xEF\xB8\x8F","\xE2\x86\x96\xEF\xB8\x8F","\xE2\x86\x95\xEF\xB8\x8F","\xE2\x86\x94\xEF\xB8\x8F","\xE2\x84\xB9\xEF\xB8\x8F",
	"\xE2\x84\xA2\xEF\xB8\x8F","\xE2\x81\x89\xEF\xB8\x8F","\xE2\x80\xBC\xEF\xB8\x8F","\xC2\xAE\xEF\xB8\x8F","\xC2\xA9\xEF\xB8\x8F",
};
#define mxCharSet_RGI_Emoji_Flag_Sequence 0
#define gxCharSet_RGI_Emoji_Flag_Sequence C_NULL
#define mxStrings_RGI_Emoji_Flag_Sequence 258
static const txString ICACHE_RODATA_ATTR gxStrings_RGI_Emoji_Flag_Sequence[mxStrings_RGI_Emoji_Flag_Sequence] = {
	"\xF0\x9F\x87\xBF\xF0\x9F\x87\xBC","\xF0\x9F\x87\xBF\xF0\x9F\x87\xB2","\xF0\x9F\x87\xBF\xF0\x9F\x87\xA6","\xF0\x9F\x87\xBE\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xBE\xF0\x9F\x87\xAA","\xF0\x9F\x87\xBD\xF0\x9F\x87\xB0","\xF0\x9F\x87\xBC\xF0\x9F\x87\xB8","\xF0\x9F\x87\xBC\xF0\x9F\x87\xAB",
	"\xF0\x9F\x87\xBB\xF0\x9F\x87\xBA","\xF0\x9F\x87\xBB\xF0\x9F\x87\xB3","\xF0\x9F\x87\xBB\xF0\x9F\x87\xAE","\xF0\x9F\x87\xBB\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xBB\xF0\x9F\x87\xAA","\xF0\x9F\x87\xBB\xF0\x9F\x87\xA8","\xF0\x9F\x87\xBB\xF0\x9F\x87\xA6","\xF0\x9F\x87\xBA\xF0\x9F\x87\xBF",
	"\xF0\x9F\x87\xBA\xF0\x9F\x87\xBE","\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8","\xF0\x9F\x87\xBA\xF0\x9F\x87\xB3","\xF0\x9F\x87\xBA\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xBA\xF0\x9F\x87\xAC","\xF0\x9F\x87\xBA\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB9\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB9\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB9\xF0\x9F\x87\xB0",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xAF","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB9\xF0\x9F\x87\xAB",
	"\xF0\x9F\x87\xB9\xF0\x9F\x87\xA9","\xF0\x9F\x87\xB9\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB9\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBF",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBD","\xF0\x9F\x87\xB8\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB8\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAF",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xAE","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB8\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xB8\xF0\x9F\x87\xA9","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA7","\xF0\x9F\x87\xB8\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xB7\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB7\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB7\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB7\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xB7\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB6\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB5\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB5\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xB9","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB5\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAD",
	"\xF0\x9F\x87\xB5\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB5\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB5\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xB4\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB3\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xB5","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAE",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAB","\xF0\x9F\x87\xB3\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB3\xF0\x9F\x87\xA8",
	"\xF0\x9F\x87\xB3\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBF","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBD",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB2\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB6","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB5",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xB4","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB2\xF0\x9F\x87\xB1",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAD","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB2\xF0\x9F\x87\xAB",
	"\xF0\x9F\x87\xB2\xF0\x9F\x87\xAA","\xF0\x9F\x87\xB2\xF0\x9F\x87\xA9","\xF0\x9F\x87\xB2\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB2\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB1\xF0\x9F\x87\xBB","\xF0\x9F\x87\xB1\xF0\x9F\x87\xBA","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB9",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xB8","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB1\xF0\x9F\x87\xB0","\xF0\x9F\x87\xB1\xF0\x9F\x87\xAE",
	"\xF0\x9F\x87\xB1\xF0\x9F\x87\xA8","\xF0\x9F\x87\xB1\xF0\x9F\x87\xA7","\xF0\x9F\x87\xB1\xF0\x9F\x87\xA6","\xF0\x9F\x87\xB0\xF0\x9F\x87\xBF",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xBE","\xF0\x9F\x87\xB0\xF0\x9F\x87\xBC","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB7","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB5",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xB3","\xF0\x9F\x87\xB0\xF0\x9F\x87\xB2","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAE","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAD",
	"\xF0\x9F\x87\xB0\xF0\x9F\x87\xAC","\xF0\x9F\x87\xB0\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAF\xF0\x9F\x87\xB5","\xF0\x9F\x87\xAF\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xAF\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAF\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB8",
	"\xF0\x9F\x87\xAE\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB6","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB4","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xAE\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAE\xF0\x9F\x87\xB1","\xF0\x9F\x87\xAE\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAE\xF0\x9F\x87\xA9",
	"\xF0\x9F\x87\xAE\xF0\x9F\x87\xA8","\xF0\x9F\x87\xAD\xF0\x9F\x87\xBA","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xAD\xF0\x9F\x87\xB3","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAD\xF0\x9F\x87\xB0","\xF0\x9F\x87\xAC\xF0\x9F\x87\xBE",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xBC","\xF0\x9F\x87\xAC\xF0\x9F\x87\xBA","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB8",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB6","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB5","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB3",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAC\xF0\x9F\x87\xB1","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAE","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAD",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xAC","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAB","\xF0\x9F\x87\xAC\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAC\xF0\x9F\x87\xA9",
	"\xF0\x9F\x87\xAC\xF0\x9F\x87\xA7","\xF0\x9F\x87\xAC\xF0\x9F\x87\xA6","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB7","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB4",
	"\xF0\x9F\x87\xAB\xF0\x9F\x87\xB2","\xF0\x9F\x87\xAB\xF0\x9F\x87\xB0","\xF0\x9F\x87\xAB\xF0\x9F\x87\xAF","\xF0\x9F\x87\xAB\xF0\x9F\x87\xAE",
	"\xF0\x9F\x87\xAA\xF0\x9F\x87\xBA","\xF0\x9F\x87\xAA\xF0\x9F\x87\xB9","\xF0\x9F\x87\xAA\xF0\x9F\x87\xB8","\xF0\x9F\x87\xAA\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xAA\xF0\x9F\x87\xAD","\xF0\x9F\x87\xAA\xF0\x9F\x87\xAC","\xF0\x9F\x87\xAA\xF0\x9F\x87\xAA","\xF0\x9F\x87\xAA\xF0\x9F\x87\xA8",
	"\xF0\x9F\x87\xAA\xF0\x9F\x87\xA6","\xF0\x9F\x87\xA9\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA9\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA9\xF0\x9F\x87\xB2",
	"\xF0\x9F\x87\xA9\xF0\x9F\x87\xB0","\xF0\x9F\x87\xA9\xF0\x9F\x87\xAF","\xF0\x9F\x87\xA9\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA9\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBE","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBD","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xBB","\xF0\x9F\x87\xA8\xF0\x9F\x87\xBA","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB7","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB5",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA8\xF0\x9F\x87\xB1",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xB0","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAE","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAD","\xF0\x9F\x87\xA8\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xA8\xF0\x9F\x87\xAB","\xF0\x9F\x87\xA8\xF0\x9F\x87\xA9","\xF0\x9F\x87\xA8\xF0\x9F\x87\xA8","\xF0\x9F\x87\xA8\xF0\x9F\x87\xA6",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBE","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBC","\xF0\x9F\x87\xA7\xF0\x9F\x87\xBB",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xB9","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB8","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB7","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB6",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB3","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA7\xF0\x9F\x87\xB1",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xAF","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAE","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAD","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAC",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xAB","\xF0\x9F\x87\xA7\xF0\x9F\x87\xAA","\xF0\x9F\x87\xA7\xF0\x9F\x87\xA9","\xF0\x9F\x87\xA7\xF0\x9F\x87\xA7",
	"\xF0\x9F\x87\xA7\xF0\x9F\x87\xA6","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBF","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBD","\xF0\x9F\x87\xA6\xF0\x9F\x87\xBC",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xBA","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB9","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB8","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB7",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xB6","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB4","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB2","\xF0\x9F\x87\xA6\xF0\x9F\x87\xB1",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xAE","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAC","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAB","\xF0\x9F\x87\xA6\xF0\x9F\x87\xAA",
	"\xF0\x9F\x87\xA6\xF0\x9F\x87\xA9","\xF0\x9F\x87\xA6\xF0\x9F\x87\xA8",
};
#define mxCharSet_RGI_Emoji_Modifier_Sequence 0
#define gxCharSet_RGI_Emoji_Modifier_Sequence C_NULL
#define mxStrings_RGI_Emoji_Modifier_Sequence 655
static const txString ICACHE_RODATA_ATTR gxStrings_RGI_Emoji_Modifier_Sequence[mxStrings_RGI_Emoji_Modifier_Sequence] = {
	"\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\xB8\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB5\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB3\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xAB\xB0\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\x85\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\x84\xF0\x9F\x8F\xBB","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x95\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x93\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x92\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA6\xBB\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\xA6\xB6\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA6\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBC","\xF0\x9F\xA5\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB6\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB3\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\xB1\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xB0\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\x9E\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9D\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9C\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x9B\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\x9A\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x99\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x98\xF0\x9F\x8F\xBB","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\xA4\x8F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA4\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\x9B\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBF","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBD","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBC","\xF0\x9F\x9B\x80\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x96\x96\xF0\x9F\x8F\xBE","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBD","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBC","\xF0\x9F\x96\x96\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x96\x95\xF0\x9F\x8F\xBF","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBE","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBD","\xF0\x9F\x96\x95\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x96\x95\xF0\x9F\x8F\xBB","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBF","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBE","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x96\x90\xF0\x9F\x8F\xBC","\xF0\x9F\x96\x90\xF0\x9F\x8F\xBB","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBF","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBC","\xF0\x9F\x95\xBA\xF0\x9F\x8F\xBB","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBF","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x95\xB4\xF0\x9F\x8F\xBB","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBF","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBE","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBC","\xF0\x9F\x92\xAA\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x92\x91\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x91\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x8F\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x92\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x85\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x92\x83\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xBC\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB6\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB5\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB4\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB2\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAD\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAC\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xAB\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBF","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA7\xF0\x9F\x8F\xBB","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBE","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBD","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBC","\xF0\x9F\x91\xA6\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x90\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x90\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x90\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8F\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8E\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x8A\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x89\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x89\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x88\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x88\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x88\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\x87\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\x86\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x86\xF0\x9F\x8F\xBB","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\x83\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x91\x83\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\x82\xF0\x9F\x8F\xBF","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x91\x82\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\x82\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x8F\x87\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBE","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBD","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBC","\xF0\x9F\x8F\x82\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBF","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBE","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBD","\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x8E\x85\xF0\x9F\x8F\xBB","\xE2\x9C\x8D\xF0\x9F\x8F\xBF","\xE2\x9C\x8D\xF0\x9F\x8F\xBE","\xE2\x9C\x8D\xF0\x9F\x8F\xBD","\xE2\x9C\x8D\xF0\x9F\x8F\xBC",
	"\xE2\x9C\x8D\xF0\x9F\x8F\xBB","\xE2\x9C\x8C\xF0\x9F\x8F\xBF","\xE2\x9C\x8C\xF0\x9F\x8F\xBE","\xE2\x9C\x8C\xF0\x9F\x8F\xBD","\xE2\x9C\x8C\xF0\x9F\x8F\xBC",
	"\xE2\x9C\x8C\xF0\x9F\x8F\xBB","\xE2\x9C\x8B\xF0\x9F\x8F\xBF","\xE2\x9C\x8B\xF0\x9F\x8F\xBE","\xE2\x9C\x8B\xF0\x9F\x8F\xBD","\xE2\x9C\x8B\xF0\x9F\x8F\xBC",
	"\xE2\x9C\x8B\xF0\x9F\x8F\xBB","\xE2\x9C\x8A\xF0\x9F\x8F\xBF","\xE2\x9C\x8A\xF0\x9F\x8F\xBE","\xE2\x9C\x8A\xF0\x9F\x8F\xBD","\xE2\x9C\x8A\xF0\x9F\x8F\xBC",
	"\xE2\x9C\x8A\xF0\x9F\x8F\xBB","\xE2\x9B\xB9\xF0\x9F\x8F\xBF","\xE2\x9B\xB9\xF0\x9F\x8F\xBE","\xE2\x9B\xB9\xF0\x9F\x8F\xBD","\xE2\x9B\xB9\xF0\x9F\x8F\xBC",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBB","\xE2\x98\x9D\xF0\x9F\x8F\xBF","\xE2\x98\x9D\xF0\x9F\x8F\xBE","\xE2\x98\x9D\xF0\x9F\x8F\xBD","\xE2\x98\x9D\xF0\x9F\x8F\xBC",
	"\xE2\x98\x9D\xF0\x9F\x8F\xBB",
};
#define mxCharSet_RGI_Emoji_Tag_Sequence 0
#define gxCharSet_RGI_Emoji_Tag_Sequence C_NULL
#define mxStrings_RGI_Emoji_Tag_Sequence 3
static const txString ICACHE_RODATA_ATTR gxStrings_RGI_Emoji_Tag_Sequence[mxStrings_RGI_Emoji_Tag_Sequence] = {
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB7\xF3\xA0\x81\xAC\xF3\xA0\x81\xB3\xF3\xA0\x81\xBF",
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF",
	"\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xA5\xF3\xA0\x81\xAE\xF3\xA0\x81\xA7\xF3\xA0\x81\xBF",
};
#define mxCharSet_RGI_Emoji_ZWJ_Sequence 0
#define gxCharSet_RGI_Emoji_ZWJ_Sequence C_NULL
#define mxStrings_RGI_Emoji_ZWJ_Sequence 1350
static const txString ICACHE_RODATA_ATTR gxStrings_RGI_Emoji_ZWJ_Sequence[mxStrings_RGI_Emoji_ZWJ_Sequence] = {
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA9",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x92\x8B\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA9",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x91\xA8",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBB",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBF","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBE",
	"\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBD","\xF0\x9F\xAB\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xAB\xB2\xF0\x9F\x8F\xBC",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x99\x85\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x95\xB5\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x95\xB5\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\x81\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x97\xA8\xEF\xB8\x8F","\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xE2\x9A\xA7\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8C\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8C\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8B\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8B\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x8A\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xE2\x9B\xB9\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xE2\x9B\xB9\xEF\xB8\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9C\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9C\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9B\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9B\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x9A\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x9A\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x99\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x99\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x98\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x98\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x97\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x97\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x96\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x96\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x94\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\xA7\x94\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F","\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F","\xF0\x9F\xA7\x8F\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8F\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA7\x8D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA7\x8D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB9\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB9\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA6\xB8\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA6\xB8\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBD\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBD\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xBC\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xBC\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB9\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB9\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB8\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB8\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB7\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB7\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xB5\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xB5\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\xA4\xA6\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\xA4\xA6\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB6\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB5\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB5\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xB4\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xB4\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x9A\xA3\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x9A\xA3\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x8E\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8E\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x8D\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8D\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x8B\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x8B\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x87\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x87\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x86\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x86\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x99\x85\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x99\x85\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x98\xB6\xE2\x80\x8D\xF0\x9F\x8C\xAB\xEF\xB8\x8F",
	"\xF0\x9F\x92\x87\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x87\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x86\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x86\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x82\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x82\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x92\x81\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x92\x81\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB7\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB7\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB3\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB3\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB1\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB1\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xB0\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xB0\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAF\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAF\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xAE\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F","\xF0\x9F\x91\xAE\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA9\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA9\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBF\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBE\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBD\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\xA6\xAF",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x92\xBB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBC\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB2",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xAC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8D\xB3",
	"\xF0\x9F\x91\xA8\xF0\x9F\x8F\xBB\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9C\x88\xEF\xB8\x8F",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9A\x96\xEF\xB8\x8F","\xF0\x9F\x91\xA8\xE2\x80\x8D\xE2\x9A\x95\xEF\xB8\x8F",
	"\xF0\x9F\x90\xBB\xE2\x80\x8D\xE2\x9D\x84\xEF\xB8\x8F","\xF0\x9F\x8F\xB4\xE2\x80\x8D\xE2\x98\xA0\xEF\xB8\x8F",
	"\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x8C\x88","\xF0\x9F\x8F\x8A\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x8A\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x8F\x84\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x84\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x82\xEF\xB8\x8F",
	"\xF0\x9F\x8F\x83\xE2\x80\x8D\xE2\x99\x80\xEF\xB8\x8F","\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\xA9\xB9",
	"\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x94\xA5","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8F\xAD",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\xA4",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8E\x84","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x98\xB5\xE2\x80\x8D\xF0\x9F\x92\xAB",
	"\xF0\x9F\x98\xAE\xE2\x80\x8D\xF0\x9F\x92\xA8","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xBC",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB3","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB1",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xB0","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x9A\x92",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x9A\x80","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x94\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBC","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8F\xAB",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\xA8","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8E\x93",
	"\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8D\xBC","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x8C\xBE",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBD","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xBC","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB3",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB2","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB1","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xB0",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\xA6\xAF","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x9A\x92","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x9A\x80",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x94\xAC","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x94\xA7","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x92\xBC",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x92\xBB","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA7","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA6",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8F\xAD","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8F\xAB","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\xA8",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\xA4","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8E\x93","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8D\xBC",
	"\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8D\xB3","\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x8C\xBE","\xF0\x9F\x90\xA6\xE2\x80\x8D\xE2\xAC\x9B",
	"\xF0\x9F\x90\x95\xE2\x80\x8D\xF0\x9F\xA6\xBA","\xF0\x9F\x90\x88\xE2\x80\x8D\xE2\xAC\x9B",
};
#define mxCharSet_String_Property 7
static const txCharSetUnicodeStringProperty ICACHE_RODATA_ATTR gxCharSet_String_Property[mxCharSet_String_Property] = {
	{ "Basic_Emoji", mxCharSet_Basic_Emoji, gxCharSet_Basic_Emoji, mxStrings_Basic_Emoji, gxStrings_Basic_Emoji },
	{ "Emoji_Keycap_Sequence", mxCharSet_Emoji_Keycap_Sequence, gxCharSet_Emoji_Keycap_Sequence, mxStrings_Emoji_Keycap_Sequence, gxStrings_Emoji_Keycap_Sequence },
	{ "RGI_Emoji", mxCharSet_RGI_Emoji, gxCharSet_RGI_Emoji, mxStrings_RGI_Emoji, gxStrings_RGI_Emoji },
	{ "RGI_Emoji_Flag_Sequence", mxCharSet_RGI_Emoji_Flag_Sequence, gxCharSet_RGI_Emoji_Flag_Sequence, mxStrings_RGI_Emoji_Flag_Sequence, gxStrings_RGI_Emoji_Flag_Sequence },
	{ "RGI_Emoji_Modifier_Sequence", mxCharSet_RGI_Emoji_Modifier_Sequence, gxCharSet_RGI_Emoji_Modifier_Sequence, mxStrings_RGI_Emoji_Modifier_Sequence, gxStrings_RGI_Emoji_Modifier_Sequence },
	{ "RGI_Emoji_Tag_Sequence", mxCharSet_RGI_Emoji_Tag_Sequence, gxCharSet_RGI_Emoji_Tag_Sequence, mxStrings_RGI_Emoji_Tag_Sequence, gxStrings_RGI_Emoji_Tag_Sequence },
	{ "RGI_Emoji_ZWJ_Sequence", mxCharSet_RGI_Emoji_ZWJ_Sequence, gxCharSet_RGI_Emoji_ZWJ_Sequence, mxStrings_RGI_Emoji_ZWJ_Sequence, gxStrings_RGI_Emoji_ZWJ_Sequence },
};

int fxCharSetUnicodePropertyCompare(const void *id, const void *it)
{
	return c_strcmp((char*)id, ((txCharSetUnicodeProperty*)it)->id);
}

void* fxCharSetUnicodeProperty(txPatternParser* parser)
{
	txString name = NULL;
	txString value = NULL;
	txString p = parser->error;
	txString q = p + sizeof(parser->error) - 1;
	txInteger c;
	txCharSetUnicodeProperty* it = NULL;
	txBoolean isStringProperty = 0;
	txCharSet* result = NULL;
	
	fxPatternParserNext(parser);
	c = parser->character;
	if (c != '{')
		fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	fxPatternParserNext(parser);
	c = parser->character;
	name = p;
	while ((('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z')) || (c == '_') || (('0' <= c) && (c <= '9'))) {
		if (p == q)
			fxPatternParserError(parser, gxErrors[mxNameOverflow]);			
		*p++ = (char)c;
		fxPatternParserNext(parser);
		c = parser->character;
	}
	*p = 0;
	if (c == '=') {
		if (p == q)
			fxPatternParserError(parser, gxErrors[mxNameOverflow]);			
		p++;
		fxPatternParserNext(parser);
		c = parser->character;
		value = p;
		while ((('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z')) || (c == '_') || (('0' <= c) && (c <= '9'))) {
			if (p == q)
				fxPatternParserError(parser, gxErrors[mxNameOverflow]);			
			*p++ = (char)c;
			fxPatternParserNext(parser);
			c = parser->character;
		}
		*p = 0;
	}
	if (c != '}')
		fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	if (value) {
		if (!c_strcmp(name, "General_Category") || !c_strcmp(name, "gc"))
			it = (txCharSetUnicodeProperty*)bsearch(value, gxCharSet_General_Category, mxCharSet_General_Category, sizeof(txCharSetUnicodeProperty), fxCharSetUnicodePropertyCompare);
		else if (!c_strcmp(name, "Script") || !c_strcmp(name, "sc"))
			it = (txCharSetUnicodeProperty*)bsearch(value, gxCharSet_Script, mxCharSet_Script, sizeof(txCharSetUnicodeProperty), fxCharSetUnicodePropertyCompare);
		else if (!c_strcmp(name, "Script_Extensions") || !c_strcmp(name, "scx"))
			it = (txCharSetUnicodeProperty*)bsearch(value, gxCharSet_Script_Extensions, mxCharSet_Script_Extensions, sizeof(txCharSetUnicodeProperty), fxCharSetUnicodePropertyCompare);
	}
	else {
		it = (txCharSetUnicodeProperty*)bsearch(name, gxCharSet_General_Category, mxCharSet_General_Category, sizeof(txCharSetUnicodeProperty), fxCharSetUnicodePropertyCompare);
		if (!it)
			it = (txCharSetUnicodeProperty*)bsearch(name, gxCharSet_Binary_Property, mxCharSet_Binary_Property, sizeof(txCharSetUnicodeProperty), fxCharSetUnicodePropertyCompare);
		if (parser->flags & XS_REGEXP_V) {
			if (!it) {
				it = (txCharSetUnicodeProperty*)bsearch(name, gxCharSet_String_Property, mxCharSet_String_Property, sizeof(txCharSetUnicodeStringProperty), fxCharSetUnicodePropertyCompare);
				isStringProperty = 1;
			}
		}
	}
	if (!it)
		fxPatternParserError(parser, gxErrors[mxInvalidEscape]);
	
	c = it->length * sizeof(txInteger);
	result = fxPatternParserCreateTerm(parser, sizeof(txCharSet) + c, fxCharSetMeasure);
	if (isStringProperty) {
		result->stringCount = ((txCharSetUnicodeStringProperty*)it)->stringCount;
		result->strings = (txString*)(((txCharSetUnicodeStringProperty*)it)->strings);
	}
	else {
		result->stringCount = 0;
		result->strings = C_NULL;
	}
	result->characters[0] = it->length;
	if (c)
		c_memmove(&result->characters[1], it->data, c);
	return result;
}

#else

void* fxCharSetUnicodeProperty(txPatternParser* parser)
{
	fxPatternParserError(parser, gxErrors[mxUnicodePropertyEscapeNotBuiltIn]);
	return NULL;
}

#endif

static txBoolean fxIsInCharSet(txInteger character, const txInteger* base, txInteger count)
{
	txInteger min = 0;
	txInteger max = count >> 1;
	while (min < max) {
		txInteger mid = (min + max) >> 1;
		txInteger begin = *(base + (mid << 1));
		txInteger end = *(base + (mid << 1) + 1);
		if (character < begin)
			max = mid;
		else if (character >= end)
			min = mid + 1;
		else
			return 1;
	}
	return 0;
}

txInteger fxFinalSigmaToLower(txMachine* the, txInteger where, txInteger character)
{
	txString s = mxThis->value.string;
	txInteger offset = where;
	for (;;) {
		txInteger former = fxFindCharacter(s, offset, -1, XS_REGEXP_UV);
		if (former == offset)
			return 0x03c3;
		mxStringByteDecode(s + former, &character);
		if (!fxIsInCharSet(character, gxCharSet_Binary_Property_Case_Ignorable, mxCharSet_Binary_Property_Case_Ignorable))
            break;
		offset = former;
	}
    if (!fxIsInCharSet(character, gxCharSet_Binary_Property_Cased, mxCharSet_Binary_Property_Cased))
        return 0x03c3;
    s += where + 2;
    for (;;) {
		s = mxStringByteDecode(s, &character);
    	if (character == C_EOF)
    		return 0x03c2;
		if (!fxIsInCharSet(character, gxCharSet_Binary_Property_Case_Ignorable, mxCharSet_Binary_Property_Case_Ignorable))
            break;
    }
	if (!fxIsInCharSet(character, gxCharSet_Binary_Property_Cased, mxCharSet_Binary_Property_Cased))
		return 0x03c2;
    return 0x03c3;
}

#ifdef mxRun

static txInteger fx_String_prototype_toCase_aux(txMachine* the, txString* q, txString* r, txInteger length, txInteger delta)
{
	if (delta > 0) {
		txSize qo = mxPtrDiff(*q - mxThis->value.string);
		txSize ro = mxPtrDiff(*r - mxResult->value.string);
		txInteger sum = fxAddChunkSizes(the, length, delta);
		txString string = fxRenewChunk(the, mxResult->value.string, sum);
		if (!string) {
			string = (txString)fxNewChunk(the, sum);
			c_memcpy(string, mxResult->value.string, length);
		}
		*q = mxThis->value.string + qo;
		*r = string + ro;
		mxResult->value.string = string;
		return sum;
	}
	return length;
}

void fx_String_prototype_toCase(txMachine* the, txBoolean flag)
{
	txString string = mxThis->value.string;
	txInteger stringLength = mxStringLength(string);
	mxMeterSome(fxUnicodeLength(string));
	if (stringLength) {
		txString p, r;
		txInteger c;
		const txCharCase* index0 = flag ? gxCharCaseToUpper0 : gxCharCaseToLower0;
		txInteger count0 = flag ? mxCharCaseToUpper0Count : mxCharCaseToLower0Count;
		const txCharCase* index1 = flag ? gxCharCaseToUpper1 : gxCharCaseToLower1;
		txInteger count1 = flag ? mxCharCaseToUpper1Count : mxCharCaseToLower1Count;
		txCharCase* it;
		const txConditionalCharCase* conditionals = flag ? gxConditionalCharCaseToUpper : gxConditionalCharCaseToLower;
		const txInteger* specials = flag ? gxSpecialCharCaseToUpper : gxSpecialCharCaseToLower;
		stringLength++;
		mxResult->value.string = fxNewChunk(the, stringLength);
		mxResult->kind = XS_STRING_KIND;
		p = mxThis->value.string;
		r = mxResult->value.string;
		for (;;) {
			txString q = mxStringByteDecode(p, &c);
			if (c == C_EOF)
				break;
			if (c < 0x80) {
				it = (txCharCase*)index0;
				if (c < it->code)
					it = C_NULL;
				else if ((it->code + it->count) <= c)
					it = C_NULL;
			}
			else if (c < 0x10000) {
				txCharCase charCase;
				charCase.code = (txU2)c;
				charCase.count = 1;
				it = (txCharCase*)bsearch(&charCase, index0, count0, sizeof(txCharCase), fxCharCaseCompare);
			}
			else {
				txCharCase charCase;
				charCase.code = (txU2)(c & 0xFFFF);
				charCase.count = 1;
				it = (txCharCase*)bsearch(&charCase, index1, count1, sizeof(txCharCase), fxCharCaseCompare);
			}
			if (it) {
				txU1 operand = it->operand;
				txInteger specialCount = operand & 0x0F;
				if (specialCount == 0) {
					txInteger d;
					if ((operand & 0x10) && ((c & 1) == 0))
						d = c;
					else if ((operand & 0x20) && (c & 1))
						d = c;
					else if (operand & 0x40)
						d = c + it->delta;
					else if (operand & 0x80)
						d = c - it->delta;
					else
						d = (*conditionals[it->delta])(the, mxPtrDiff(p - mxThis->value.string), c);
					stringLength = fx_String_prototype_toCase_aux(the, &q, &r, stringLength, mxStringByteLength(d) - mxPtrDiff(q - p));
					r = mxStringByteEncode(r, d);
				}
				else {
					const txInteger* special = specials + it->delta;
					txInteger specialIndex = specialCount;
					txInteger specialLength = 0;
					while (specialIndex) {
						specialLength += mxStringByteLength(*special++);
						specialIndex--;
					}
					stringLength = fx_String_prototype_toCase_aux(the, &q, &r, stringLength, specialLength - mxPtrDiff(q - p));
					special = specials + it->delta;
					specialIndex = specialCount;
					while (specialIndex) {
						r = mxStringByteEncode(r, *special++);
						specialIndex--;
					}
				}
			}
			else {
				r = mxStringByteEncode(r, c);
			}
			p = q;
		}
		*r = 0;
	}
	else {
		mxResult->value.string = mxEmptyString.value.string;
		mxResult->kind = mxEmptyString.kind;
	}
}

#endif
