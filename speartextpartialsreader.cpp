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



//#include "stdafx.h"
#include "speartextpartialsreader.h"

//#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
using namespace std;

SpearTextPartialsReader::SpearTextPartialsReader(const char* filename)
{
	/*
	FILE* pFILE = fopen(filename, "r");
	if(pFILE)
	{

		fclose(pFILE);
	}
	*/
	string line;
	ifstream myfile(filename);
	int linecount=0;
	int partialscount=0;
	int partialpointscount=0;
	if (myfile)  // same as: if (myfile.good())
	{
		while (getline( myfile, line ))  // same as: while (getline( myfile, line ).good())
		{
			linecount++;
			if(linecount==1)
			{
				//skip line, do nothing
			}
			else if(linecount==2)
			{
				//skip line, do nothing
			}
			else if(linecount==3)
			{
				//read partials count
				istringstream buf(line);
				istream_iterator<string> beg(buf), end;
				vector<string> tokens(beg, end);
				int tokencount=0;
				for(vector<string>::iterator it=tokens.begin(), end=tokens.end(); it!=end; ++it)
				{
					tokencount++;
					if(tokencount==1) 
					{
						//skip
						continue;
					}
					else if(tokencount==2)
					{
						partialscount = atoi((*it).c_str());
						std::cout << *it << "\n";
					}
				}

			}
			else if(linecount==4)
			{
				//skip line, do nothing
			}
			else
			{
				//read partials data
				if(linecount % 2)
				{
					//partial header
					partialpointscount=0;
					//read partial points count
					istringstream buf(line);
					istream_iterator<string> beg(buf), end;
					vector<string> tokens(beg, end);
					int tokencount=0;
					for(vector<string>::iterator it=tokens.begin(), end=tokens.end(); it!=end; ++it)
					{
						tokencount++;
						if(tokencount==1) 
						{
							//skip
							continue;
						}
						else if(tokencount==2)
						{
							partialpointscount = atoi((*it).c_str());
							//std::cout << *it << "\n";
							break;
						}
					}

				}
				else
				{
					//partial data

					istringstream buf(line);
					istream_iterator<string> beg(buf), end;
					vector<string> tokens(beg, end);
					int tokencount=0;
					float frequency_hz=0.0f;
					float partialpointsfrequencysummation=0.0f;
					float averagefrequency_hz=0.0f;
					float amplitude=0.0f;
					float partialpointsamplitudesummation=0.0f;
					float averageamplitude=0.0f;
					for(vector<string>::iterator it=tokens.begin(), end=tokens.end(); it!=end; ++it)
					{
						tokencount++;
						if(tokencount==1) 
						{
							//skip time
							continue;
						}
						else if(tokencount==2)
						{
							//read freq
							frequency_hz = atof((*it).c_str());
							partialpointsfrequencysummation += frequency_hz;
							//std::cout << *it << "\n";
						}
						else if(tokencount==3)
						{
							//read freq
							amplitude = atof((*it).c_str());
							partialpointsamplitudesummation += amplitude;
							//std::cout << *it << "\n";
							tokencount=0;
						}
					}
					averagefrequency_hz = partialpointsfrequencysummation/partialpointscount;
					partialfrequencies.push_back(averagefrequency_hz);
					//std::cout << averagefrequency_hz << "\n";
					averageamplitude = partialpointsamplitudesummation/partialpointscount;
					partialamplitudes.push_back(averageamplitude);
					//std::cout << averageamplitude << "\n";
				}
				//std::cout << linecount << "\n";
			}
			//cout << line << "\n";
		}
		myfile.close();
		std::cout << partialfrequencies.size() << "\n";
		std::cout << partialamplitudes.size() << "\n";
		std::cout << partialscount << "\n";
		
	}
	else std::cout << "can't open file " << filename << "\n";

}

