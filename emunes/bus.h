#pragma once
#include <cstdint>
#include <array>

#include "processor.h"
#include "graphics.h"
#include "cartridge.h"

class bus
{
public:
	bus();
	~bus();

public:
	processor cpu;
	graphics ppu;

	std::shared_ptr<cartridge> cart;
	uint8_t cpuRam[2048];
	uint8_t controller[2];

public:
	void    cpuWrite(uint16_t addr, uint8_t data);
	uint8_t cpuRead(uint16_t addr, bool b = false);

private:
	uint32_t nSystemClockCounter = 0;
	uint8_t controller_state[2];

private:
	uint8_t dma_page = 0x00;
	uint8_t dma_addr = 0x00;
	uint8_t dma_data = 0x00;
	bool dma_dummy = true;

	bool dma_transfer = false;

public:
	void insertCartridge(const std::shared_ptr<cartridge>& cartridge);
	void reset();
	void clock();
};