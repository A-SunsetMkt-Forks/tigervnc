/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011 D. R. Commander.  All Rights Reserved.
 * Copyright 2014-2019 Pierre Ossman for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdexcept>

#include <core/LogWriter.h>
#include <core/string.h>

#include <rfb/ledStates.h>
#include <rfb/Cursor.h>
#include <rfb/ScreenSet.h>
#include <rfb/ServerParams.h>

using namespace rfb;

static core::LogWriter vlog("ServerParams");

ServerParams::ServerParams()
  : majorVersion(0), minorVersion(0),
    supportsQEMUKeyEvent(false),
    supportsSetDesktopSize(false), supportsFence(false),
    supportsContinuousUpdates(false), supportsExtendedMouseButtons(false),
    width_(0), height_(0),
    ledState_(ledUnknown)
{
  setName("");

  screenLayout_ = new ScreenSet();

  pf_ = new PixelFormat();

  cursor_ = new Cursor(0, 0, {}, nullptr);

  clipFlags = 0;
  memset(clipSizes, 0, sizeof(clipSizes));
}

ServerParams::~ServerParams()
{
  delete cursor_;
}

void ServerParams::setDimensions(int width, int height)
{
  ScreenSet layout;
  layout.add_screen(rfb::Screen(0, 0, 0, width, height, 0));
  setDimensions(width, height, layout);
}

void ServerParams::setDimensions(int width, int height, const ScreenSet& layout)
{
  if (!layout.validate(width, height)) {
    char buffer[2048];
    vlog.debug("Invalid screen layout for %dx%d:", width, height);
    layout.print(buffer, sizeof(buffer));
    vlog.debug("%s", buffer);

    throw std::invalid_argument("Attempted to configure an invalid screen layout");
  }

  width_ = width;
  height_ = height;
  delete screenLayout_;
  screenLayout_ = new ScreenSet(layout);
}

void ServerParams::setPF(const PixelFormat& pf)
{
  delete pf_;
  pf_ = new PixelFormat(pf);

  if (pf.bpp != 8 && pf.bpp != 16 && pf.bpp != 32)
    throw std::invalid_argument("setPF: Not 8, 16 or 32 bpp?");
}

void ServerParams::setName(const char* name)
{
  name_ = name;
}

void ServerParams::setCursor(const Cursor& other)
{
  delete cursor_;
  cursor_ = new Cursor(other);
}

void ServerParams::setLEDState(unsigned int state)
{
  ledState_ = state;
}

uint32_t ServerParams::clipboardSize(unsigned int format) const
{
  int i;

  for (i = 0;i < 16;i++) {
    if (((unsigned)1 << i) == format)
      return clipSizes[i];
  }

  throw std::invalid_argument(
    core::format("Invalid clipboard format 0x%x", format));
}

void ServerParams::setClipboardCaps(uint32_t flags, const uint32_t* lengths)
{
  int i, num;

  clipFlags = flags;

  num = 0;
  for (i = 0;i < 16;i++) {
    if (!(flags & (1 << i)))
      continue;
    clipSizes[i] = lengths[num++];
  }
}
