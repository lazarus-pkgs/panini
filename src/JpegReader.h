/* -*- c-basic-offset: 4 -*- */
/*
 * This file is part of the freepv panoramic viewer.
 *
 *  Author: Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: JpegReader.h 61 2006-10-07 23:35:15Z dangelo $
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; version 2.1 of
 * the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

#ifndef FPV_JPEGREADER_H
#define FPV_JPEGREADER_H

#include "Image.h"

namespace FPV
{
    
/** decode a jpeg image stored in \p buffer into \p image 
 *  If \p rot90 = true, the returned image is rotated 90 degrees in clockwise direction
 */
bool decodeJPEG(unsigned char * buffer, size_t buf_len, Image & image, bool rot90=false);

/** decode a jpeg image stored in \p buffer into \p image 
 *  If \p rot90 = true, the returned image is rotated 90 degrees in clockwise direction
 */
bool decodeJPEG(FILE *f, Image & image, bool rot90=false);

} // namespace

#endif
