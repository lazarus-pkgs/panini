/* -*- c-basic-offset: 4 -*- */
/*
 * This file is part of the freepv panoramic viewer.
 *
 *  Author: Fulvio Senore
 *
 *  $Id: Parameters.h 133 2008-03-02 00:07:04Z brunopostle $
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


#ifndef __FPV_PARAMETERS_H__
#define __FPV_PARAMETERS_H__

#include <string>

namespace FPV
{


/** quality of rendering */
enum RenderQuality {RQ_LOW, RQ_MEDIUM, RQ_HIGH};   

/**

this class contains the values of the viewer parameters.

To insulate from the caller each string value is copied to a new local buffer, so for each new
string parameter:
1 - allocate space when parsing
2 - free the space in the destructor
3 - if it is an URL of data to be downloaded add it to the DownloadableURLs enum

remember to initialize each parameter in InitializeParameters()

*/
class Parameters
{
public:
    /** possible panorama types */
    enum PanoType { PANO_UNKNOWN, PANO_CUBIC, PANO_QTVR, PANO_SPHERICAL, PANO_CYLINDRICAL};

	// creates an empty object, used by the OCX version
	Parameters();

	// used by Netscape plugins
	void parse( int argc, char* argn[], char* argv[] );

    // used by standalone version
    void parse( int argc, char* argv[] );

    // parses a single parameter
    void parse(const char * pname, const char * value);

    // parse a single parameter in name=value style
    void parse(const char * para);

	~Parameters(void);

private:

	int m_width, m_height;	// size of the viewer

    std::string m_src;	           // main file to be downloaded
    std::string m_sw_src;          // the main file for shockwave applications
    std::string m_path;            // this is the path where the main file is
    std::string m_waitImage;	   // url of an optional wait image
    std::string m_cubeSrc[6];      // urls of the cube images.
    RenderQuality m_renderQuality; // quality of the rendering
    PanoType m_panoType;           // type of the main panorama
    double m_panoHFOV;             // HFOV of the image, useful for partial
                                   // cylindrical and spherical images
    size_t m_maxTexMem;            // maximum texture memory

public:


    int getWidth(void) const { return m_width; }
    int getHeight(void) const { return m_height; }

    //! returns the URL of the main data file (panoramic image)
    const std::string & getSrc(void) const { return m_src; }

    //! returns the schockwave URL of the main data file
    const std::string & get_SW_Src(void) const { return m_sw_src; }

    //! returns the path of the main file
    const std::string & getPath() const{ return m_path; }

    //! returns the URL of an optional wait image
    const std::string & getWaitImage(void) const { return m_waitImage; }

    //! returns the URL of a cube face
    const std::string & getCubeSrc(int i) const { return m_cubeSrc[i]; }

    RenderQuality getRenderQuality() const { return m_renderQuality; }

    PanoType getPanoType() const { return m_panoType; };

    double getPanoHFOV() const { return m_panoHFOV; }

    size_t getMaxTexMem() const { return m_maxTexMem; }

private:
	// sets an initial value to the parameters
	void initializeParameters(void);
	// // allocates a buffer for a string parameter, copies it an return the buffer address
	char * copyStringParameter(const char * value);
	// returns the int value of a parameter
	int copyIntParameter(const char * value);
};


} // namespace

#endif
