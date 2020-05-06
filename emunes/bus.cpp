#include "bus.h"

bus::bus()
{
	cpu.ConnectBus(this);
}

bus::~bus()
{}

void bus::cpuWrite(uint16_t addr, uint8_t data)
{
	if (cart->cpuWrite(addr, data))
	{}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		cpuRam[addr & 0x07FF] = data;
	}
	else if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		ppu.cpuWrite(addr & 0x0007, data);
	}
	else if (addr == 0x4014)
	{
		dma_page = data;
		dma_addr = 0x00;
		dma_transfer = true;
	}
	else if (addr >= 0x4016 && addr <= 0x4017)
	{
		controller_state[addr & 0x0001] = controller[addr & 0x0001];
	}
}

uint8_t bus::cpuRead(uint16_t addr, bool b)
{
	uint8_t data = 0x00;
	if (cart->cpuRead(addr, data))
	{}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		data = cpuRam[addr & 0x07FF];
	}
	else if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		data = ppu.cpuRead(addr & 0x0007, b);
	}
	else if (addr >= 0x4016 && addr <= 0x4017)
	{
		data = (controller_state[addr & 0x0001] & 0x80) > 0;
		controller_state[addr & 0x0001] <<= 1;
	}

	return data;
}

void bus::insertCartridge(const std::shared_ptr<cartridge>& cartridge)
{
	this->cart = cartridge;
	ppu.ConnectCartridge(cartridge);
}

void bus::reset()
{
	cart->reset();
	cpu.reset();
	ppu.reset();
	nSystemClockCounter = 0;
	dma_page = 0x00;
	dma_addr = 0x00;
	dma_data = 0x00;
	dma_dummy = true;
	dma_transfer = false;
}

void bus::clock()
{
	ppu.clock();
	if (nSystemClockCounter % 3 == 0)
	{
		if (dma_transfer)
		{
			if (dma_dummy)
			{
				if (nSystemClockCounter % 2 == 1)
				{
					dma_dummy = false;
				}
			}
			else
			{
				if (!(nSystemClockCounter & 1))
				{
					dma_data = cpuRead(dma_page << 8 | dma_addr);
				}
				else
				{
					ppu.pOAM[dma_addr] = dma_data;
					dma_addr++;
					if (dma_addr == 0x00)
					{
						dma_transfer = false;
						dma_dummy = true;
					}
				}
			}
		}
		else
		{
			cpu.clock();
		}
	}

	if (ppu.nmi)
	{
		ppu.nmi = false;
		cpu.nmi();
	}

	nSystemClockCounter++;
}