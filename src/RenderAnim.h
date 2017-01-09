#ifndef RENDER_ANIM_H_
#define RENDER_ANIM_H_
#include <stdio.h>
#include "LedControl.h"
#include <Arduino.h>
#include <FS.h>

#define FRAME_SIZE 32

class RenderAnim {

public:
								RenderAnim(LedControl lc);
								void reset();
								void handleRender();
								void setAnim(String fileName, bool loop, int time);
                void readFrame();
                void renderFrame();
private:
								LedControl m_lc;
								String m_fileName;
								bool m_loop;
								byte m_buffer[4][8];
								int m_time;
								unsigned long m_oldMillis;
                int m_currentFrame;
                int m_numFrames;
                File m_fileAnim;
};

#endif /* RENDER_TEXT_H_ */
