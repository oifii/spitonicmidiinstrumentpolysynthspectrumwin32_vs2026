/*
 * Copyright (c) 2012-2026 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iostream>
using std::cout; using std::endl;

#include <string>

#ifndef _SPIREGREADWRITE_H
#define _SPIREGREADWRITE_H

HKEY OpenKey(HKEY hRootKey, wchar_t* strKey);
void SetValDWORD(HKEY hKey, LPCTSTR lpValue, DWORD data);
DWORD GetValDWORD(HKEY hKey, LPCTSTR lpValue);

DWORD RegReadDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue);
void RegWriteDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, DWORD data);

std::wstring RegReadSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue);
void RegWriteSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, std::wstring data);

#endif //SPIREGREADWRITE_H