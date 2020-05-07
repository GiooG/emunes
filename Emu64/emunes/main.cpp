/*	
	License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2020 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	1. Redistributions or derivations of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions or derivative works in binary form must reproduce the above
	copyright notice. This list of conditions and the following	disclaimer must be
	reproduced in the documentation and/or other materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its contributors may
	be used to endorse or promote products derived from this software without specific
	prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
	SUCH DAMAGE.
	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Homepage:	https://www.onelonecoder.com
	Patreon:	https://www.patreon.com/javidx9
	Community:  https://community.onelonecoder.com
*/

#include <iostream>
#include <sstream>

#include "bus.h"
#include "processor.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_SOUND
#include "olcPGEX_Sound.h"

class EmuNES : public olc::PixelGameEngine
{
public:
	EmuNES() { sAppName = "EmuNES"; }

private:
	bus nes;
	std::shared_ptr<cartridge> cart;
	bool bEmulationRun = true;
	float fResidualTime = 0.0f;

	uint8_t nSelectedPalette = 0x00;

	std::list<uint16_t> audio[4];
	float fAccumulatedTime = 0.0f;

private:
	std::map<uint16_t, std::string> mapAsm;

	std::string hex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	static EmuNES* pInstance;

	static float SoundOut(int nChannel, float fGlobalTime, float fTimeStep)
	{
		if (nChannel == 0)
		{
			while (!pInstance->nes.clock()) {};
			return static_cast<float>(pInstance->nes.dAudioSample);
		}
		else
			return 0.0f;
	}

	bool OnUserCreate() override
	{
		std::string rom;
		std::cout << "Enter rom name including the .nes: ";
		std::cin >> rom;
		cart = std::make_shared<cartridge>("roms/"+rom);

		if (!cart->imageValid())
			return false;

		nes.insertCartridge(cart);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 120; j++)
				audio[i].push_back(0);
		}
		nes.reset();

		pInstance = this;

		nes.SetSampleFrequency(44100);
		olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
		olc::SOUND::SetUserSynthFunction(SoundOut);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		EmulatorUpdateWithAudio(fElapsedTime);
		return true;
	}

	bool EmulatorUpdateWithAudio(float fElapsedTime)
	{
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= 1.0f / 60.0f)
		{
			fAccumulatedTime -= (1.0f / 60.0f);
			audio[0].pop_front();
			audio[0].push_back(nes.apu.pulse1_visual);
			audio[1].pop_front();
			audio[1].push_back(nes.apu.pulse2_visual);
			audio[2].pop_front();
			audio[2].push_back(nes.apu.noise_visual);
		}

		Clear(olc::DARK_BLUE);

		nes.controller[0] = 0x00;
		nes.controller[0] |= GetKey(olc::Key::X).bHeld ? 0x80 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::Z).bHeld ? 0x40 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::A).bHeld ? 0x20 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::S).bHeld ? 0x10 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::UP).bHeld ? 0x08 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::DOWN).bHeld ? 0x04 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::LEFT).bHeld ? 0x02 : 0x00;
		nes.controller[0] |= GetKey(olc::Key::RIGHT).bHeld ? 0x01 : 0x00;

		if (GetKey(olc::Key::R).bPressed) nes.reset();
		if (GetKey(olc::Key::ESCAPE).bPressed) exit(0);

		DrawSprite(0, 0, &nes.ppu.GetScreen(), 1);
		return true;
	}
};

EmuNES* EmuNES::pInstance = nullptr;

int main()
{
	EmuNES demo;
	demo.Construct(256, 240, 3, 3);
	demo.Start();
	return 0;
}