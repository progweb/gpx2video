//==============================================================================
//
//                gpxtrk - the gpxtrk example
//
//               Copyright (C) 2013  Dick van Oudheusden
//--
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free
// Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//==============================================================================
//-
//  $Date: 2013-03-10 12:02:27 +0100 (Sun, 10 Mar 2013) $ $Revision: 5 $
//
//==============================================================================
//

#include <fstream>
#include <iostream>

#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"


using namespace std;

void show(gpx::Node &node, unsigned width)
{
  string value(width, ' ');

  value = node.getValue();

  if (value.length() > width)
  {
    value = value.substr(0, width);
  }
  else if (value.length() < width)
  {
    value.insert(0, width - value.length(), ' ');
  }

  cout << value << ' ';
}

void show(const string str, unsigned width)
{
  string value = str;;

  if (value.length() > width)
  {
    value = value.substr(0, width);
  }
  else if (value.length() < width)
  {
    value.append(width - value.length(), ' ');
  }

  cout << value << ' ';
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Usage: gpxtrk FILE ..." << endl;
    cerr << "  Show the track information in a gpx file." << endl;
    exit(1);
  }

  for (int i = 1; i < argc; i++)
  {
    ifstream stream(argv[i]);

    if (stream.is_open())
    {
      gpx::ReportCerr report;

      gpx::Parser parser(&report);

      gpx::GPX *root = parser.parse(stream);

      if (root == nullptr)
      {
        cerr << "Parsing of " << argv[i] << " failed due to " << parser.errorText() << " on line " << parser.errorLineNumber() << " and column " << parser.errorColumnNumber() << endl;
      }
      else
      {
        list<gpx::TRK*> &trks = root->trks().list();

        int trknr = 1;
        for (list<gpx::TRK*>::iterator node = trks.begin(); node != trks.end(); ++node)
        {
          gpx::TRK *trk = (*node);

          if (trk != nullptr)
          {
            cout << "Track:" << trknr++ << endl;
            cout << "  Name        : " << trk->name()  .getValue() << endl;
            cout << "  Comment     : " << trk->cmt()   .getValue() << endl;
            cout << "  Description : " << trk->desc()  .getValue() << endl;
            cout << "  Source      : " << trk->src()   .getValue() << endl;
            cout << "  Type        : " << trk->type()  .getValue() << endl;
            cout << "  Number      : " << trk->number().getValue() << endl;
            cout << "  Segments:   : " << trk->trksegs().list().size()  << endl << endl;

            list<gpx::TRKSeg*> &trksegs = trk->trksegs().list();

            int segnr = 1;
            for (list<gpx::TRKSeg*>::iterator iter2 = trksegs.begin(); iter2 != trksegs.end(); ++iter2)
            {
              gpx::TRKSeg *seg = (*iter2);

              if (seg != nullptr)
              {
                cout << "  Segment     : " << segnr++ << endl;
                cout << "    ";
                show("Latitude", 11);
                show("Longitude", 11);
                show("Elevation", 11);
                show("Time", 20);
                show("Name", 8);
                show("Comment", 10);
                show("Extensions", 10);
                cout << endl;

                list<gpx::WPT*> &trkpts = seg->trkpts().list();

                for (list<gpx::WPT*>::iterator iter3 = trkpts.begin(); iter3 != trkpts.end(); ++iter3)
                {
                  gpx::WPT *trkpt = (*iter3);

                  if (trkpt != nullptr)
                  {
                    cout << "    ";
                    show(trkpt->lat(),  11);
                    show(trkpt->lon(),  11);
                    show(trkpt->ele(),  11);
                    show(trkpt->time(), 20);
                    show(trkpt->getName(),  8);
                    show(trkpt->cmt(),  10);
                    show(trkpt->extensions().getValue(), 10);
                    cout << endl;
                  }
                }
              }
            }
          }
        }
        cout << endl;
      }

      stream.close();
    }
    else
    {
      cerr << "File: " << argv[i] << " not found." << endl;
    }
  }
  exit(0);
}
