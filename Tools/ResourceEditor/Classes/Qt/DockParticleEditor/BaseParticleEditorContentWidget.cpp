/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseParticleEditorContentWidget.h"

const QChar BaseParticleEditorContentWidget::DEGREE_MARK_CHARACTER = QChar(0x00B0);

BaseParticleEditorContentWidget::BaseParticleEditorContentWidget()
{
	emitter = NULL;
}

int BaseParticleEditorContentWidget::ConvertFromPlaybackSpeedToSliderValue(float32 playbackSpeed)
{
	playbackSpeed = Clamp(playbackSpeed, PARTICLE_EMITTER_MIN_PLAYBACK_SPEED, PARTICLE_EMITTER_MAX_PLAYBACK_SPEED);
	
	// Our scale is logarithmic.
	uint playbackSpeedInt = (uint)(playbackSpeed / PARTICLE_EMITTER_MIN_PLAYBACK_SPEED);
	uint logValue = -1;
	while (playbackSpeedInt)
	{
		logValue++;
		playbackSpeedInt >>= 1;
	}
	
	return logValue;
}

float BaseParticleEditorContentWidget::ConvertFromSliderValueToPlaybackSpeed(int sliderValue)
{
	// Our scale is logarithmic.
	uint scaleFactor = (1 << sliderValue);
	return PARTICLE_EMITTER_MIN_PLAYBACK_SPEED * scaleFactor;
}