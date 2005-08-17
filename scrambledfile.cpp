/*
 * scrambledfile.cpp
 * Encrypted file class implementation
 * Copyright (C) 2005 Christopher Han <xiphux@gmail.com>
 *
 * This file is part of hpiutil2.
 *
 * hpiutil2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * hpiutil2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hpiutil2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "scrambledfile.h"

/**
 * Constructor
 * @param file name of scrambled file to open as a c string
 */
scrambledfile::scrambledfile(const char *fname)
{
	file.open(fname,std::ios::in|std::ios::binary);
	scrambled = false;
}

/**
 * Constructor
 * @param file name of scrambled file to open as a c++ string
 */
scrambledfile::scrambledfile(std::string const &fname)
{
	file.open(fname.c_str(),std::ios::in|std::ios::binary);
	scrambled = false;
}

/**
 * Destructor
 */
scrambledfile::~scrambledfile()
{
	file.close();
}

/**
 * read
 * reads a single byte from the file
 * @return byte read
 */
uint8_t scrambledfile::read()
{
	uint32_t oldpos = file.tellg();
	char ret = file.get();
	if (scrambled && (ret != EOF))
		ret = (char)(((key ^ oldpos) ^ ~ret)&0x00ff);
	return (uint8_t)ret;
}

/**
 * read
 * reads data from the file into a buffer
 * @return number of bytes read
 * @param buf buffer to read into
 */
uint32_t scrambledfile::read(uint8_t *buf)
{
	uint32_t oldpos = (uint32_t)file.tellg();
	file.read((char*)buf,sizeof(buf));
	uint32_t read = (uint32_t)file.tellg() - oldpos;
	if (scrambled) {
		for (int i = 0; i < read; i++)
			buf[i] = (uint8_t)(((key ^ (oldpos+i)) ^ ~buf[i])&0x00ff);
	}
	return read;
}

/**
 * read
 * reads data from the file into a buffer
 * @return number of bytes read
 * @param buf buffer to read into
 * @param off offset in file to start reading from
 * @param len number of bytes to read
 */
uint32_t scrambledfile::read(uint8_t *buf, const uint32_t off, const uint32_t len)
{
	file.seekg(off);
	file.read((char*)buf,len);
	if (scrambled) {
		for (int i = 0; i < len; i++)
			buf[i] = (uint8_t)(((key ^ (i+off)) ^ ~buf[i])&0x00ff);
	}
	return ((uint32_t)file.tellg()-off);
}

/**
 * readint
 * reads a 32-bit integer from the file,
 * byte swabbing if necessary
 * @return swabbed integer
 */
uint32_t scrambledfile::readint()
{
	uint32_t a = (uint32_t)read();
	uint32_t b = (uint32_t)read();
	uint32_t c = (uint32_t)read();
	uint32_t d = (uint32_t)read();
	return (d<<24)|(c<<16)|(b<<8)|a;
}

/**
 * seek
 * seek to an offset in the file
 * @param pos offset to seek to
 */
void scrambledfile::seek(const uint32_t pos)
{
	file.seekg(pos,std::ios::beg);
}

/**
 * setkey
 * set or unset decryption key to use to unscramble
 * @param k key to use (use 0 to disable)
 */
void scrambledfile::setkey(const uint32_t k)
{
	if (k) {
		key = ~((k<<2)|(k>>6));
		scrambled = true;
	} else {
		key = 0;
		scrambled = false;
	}
}

/**
 * readstring
 * reads a string of data from the file,
 * terminated by \0 or EOF
 * @return string read
 */
std::string scrambledfile::readstring()
{
	std::string data;
	while (1) {
		char i = read();
		if ((i == 0)||(i == EOF))
			break;
		data.append(1,i);
	}
	return data;
}
