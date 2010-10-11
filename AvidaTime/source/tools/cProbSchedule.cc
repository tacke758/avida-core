/*
 *  cProbSchedule.cc
 *  Avida
 *
 *  Called "prob_schedule.cc" prior to 12/7/05.
 *  Copyright 1999-2010 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "cProbSchedule.h"

#include "cChangeList.h"
#include "cDeme.h"
#include "cMerit.h"

// The larger merits cause problems here; avoid very large or very small merits

int cProbSchedule::GetNextID()
{
  if (chart.GetTotalWeight() == 0) return -1;
  const double position = m_rng.GetDouble(chart.GetTotalWeight());
  return chart.FindPosition(position);
}


void cProbSchedule::Adjust(int item_id, const cMerit& item_merit, int deme_id)
{
  if (cChangeList *change_list = GetChangeList()) {
    change_list->MarkChange(item_id);
  }
  chart.SetWeight(item_id, item_merit.GetDouble());
}
