/* -*- c-basic-offset: 4 -*- */
/*
 * This file is part of the freepv panoramic viewer.
 *
 *  Author: Fulvio Senore
 *
 *  $Id: Parameters.cpp 133 2008-03-02 00:07:04Z brunopostle $
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "Parameters.h"
#include "utils.h"
#include "Utils/getPath.h"

using namespace FPV;

bool isStandalone;

Parameters::Parameters(void)
{
	initializeParameters();
}

// used by Netscape plugins
void Parameters::parse( int argc, char* argn[], char* argv[] ) {
    isStandalone=false;
    for( int i = 0; i < argc; i++ ) {
        parse( argn[i], argv[i] );
    }
}

// used by command line
void Parameters::parse( int argc, char* argv[] ) {
    //The first argument is the present work directory (pwd)
    //pwd is ignored by the parser
    isStandalone=true;
    for( int i = 1; i < argc; i++ ) {
	// add src, if not on commandline
        if(strchr(argv[i],'=')==NULL)
			parse("SRC", argv[i]);
		else
			parse(argv[i]);
    }

}

// parses a single parameter
void Parameters::parse(const char * para)
{
    char *p = NULL;
    p= new char[strlen(para) + 1];
    strcpy( p, para);
    char * equal = strchr(p, '=');
    if (equal == 0) {
        // not a valid parameter
        return;
    }
    if (*equal == 0 ) {
        // not a valid argument
        return;
    }
    *equal = 0;

    // parse name value pair
    parse( p, equal+1);
    delete[] p;
}


// parses a single parameter
void Parameters::parse(const char * pname, const char * pvalue)
{
    // convert to uppercase and remove whitespace
    std::string name = removeWhitespace(string2UPPER(pname));
    std::string value = removeWhitespace(pvalue);

    if( name == "HEIGHT" ) m_height = atoi( value.c_str() );
    if( name == "WIDTH" )  m_width = atoi( value.c_str() );
    if( name == "SRC" )	   m_src = value;
    if( name == "SWURL")   m_sw_src = value;
    if( name == "WAIT" )   m_waitImage = value;

    // single cube faces
    if( name == "CUBE_FRONT" )  m_cubeSrc[0] = value;
    if( name == "CUBE_RIGHT" )  m_cubeSrc[1] = value;
    if( name == "CUBE_BACK" )   m_cubeSrc[2] = value;
    if( name == "CUBE_LEFT" )   m_cubeSrc[3] = value;
    if( name == "CUBE_TOP" )    m_cubeSrc[4] = value;
    if( name == "CUBE_BOTTOM" ) m_cubeSrc[5] = value;

    if((m_src.size()>0)&&isStandalone)
	m_path=Utils::getPath(m_src.c_str());

    if( name == "QUALITY" ) {
        value = string2UPPER(value);
        if (value == "LOW") m_renderQuality = RQ_LOW;
        else if (value == "MEDIUM") m_renderQuality = RQ_MEDIUM;
        else if (value == "HIGH") m_renderQuality = RQ_HIGH;
    }

    if (name == "VRAM") m_maxTexMem = atoi(value.c_str()) * 1024 * 1024;

    if (name == "PANO_HFOV") m_panoHFOV = atof(value.c_str());
    if (name == "PANO_TYPE") {
        value = string2UPPER(value);
        if (value == "SPHERICAL") m_panoType = PANO_SPHERICAL;
        else if (value == "CYLINDRICAL") m_panoType = PANO_CYLINDRICAL;
        else if (value == "QTVR") m_panoType = PANO_QTVR;
        else if (value == "CUBIC") m_panoType = PANO_CUBIC;
    }
}

Parameters::~Parameters(void)
{
}

// sets an initial value to the parameters
void Parameters::initializeParameters(void)
{
	m_width = m_height = 0;
    m_renderQuality = RQ_HIGH;
    m_panoType = PANO_UNKNOWN;
    m_panoHFOV = 360;
    m_maxTexMem = 256 * 1024 * 1024;
    m_path="";
}

