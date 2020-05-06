#include "mapper.h"

mapper::mapper(uint8_t prgBanks, uint8_t chrBanks)
{
	nPRGBanks = prgBanks;
	nCHRBanks = chrBanks;

	reset();
}

mapper::~mapper()
{}

void mapper::reset()
{}