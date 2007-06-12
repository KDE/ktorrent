/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "StatsPluginPrefs.h"

namespace kt {

StatsPluginPrefs::StatsPluginPrefs() : PrefPageInterface(i18n("Statistics"), i18n("Statistics options"), KGlobal::iconLoader()->loadIcon("ktimemon",KIcon::NoGroup)), pmUi(0)
{	
}

StatsPluginPrefs::~StatsPluginPrefs()
{
}

bool StatsPluginPrefs::apply ()
{
	StatsPluginSettings::setUpdateChartsEveryGuiUpdates(pmUi -> GuiUpdatesSbw -> value());
	StatsPluginSettings::setGatherDataEveryMs(pmUi -> DataIvalSbw -> value());
	StatsPluginSettings::setPeersSpeedDataIval(pmUi -> PeersSpdUpdIvalSbw -> value());
	
	StatsPluginSettings::setPeersSpeed(pmUi -> PeersSpdCbw -> isChecked());
	StatsPluginSettings::setDrawSeedersInSwarms(pmUi -> ConnSdrInSwaCbw -> isChecked());
	StatsPluginSettings::setDrawLeechersInSwarms(pmUi -> ConnLchInSwaCbw -> isChecked());
	
	StatsPluginSettings::setDownloadMeasurements(pmUi -> DownloadMrmtSbw -> value());
	StatsPluginSettings::setPeersSpeedMeasurements(pmUi -> PeersSpdMrmtSbw -> value());
	StatsPluginSettings::setUploadMeasurements(pmUi -> UploadMrmtSbw -> value());
	StatsPluginSettings::setConnectionsMeasurements(pmUi -> ConnsMrmtSbw -> value());
	StatsPluginSettings::setDHTMeasurements(pmUi -> DHTMrmtSbw -> value());
	StatsPluginSettings::setMaxSpdMode(pmUi -> MaxSpdModeCbw -> currentItem());
	
	StatsPluginSettings::writeConfig();
	
	emit Applied();
	
	return true;
}

void StatsPluginPrefs::createWidget (QWidget *parent)
{
	pmUi = new StatsPluginPrefsPage(parent);
}

void StatsPluginPrefs::updateData ()
{
	pmUi -> GuiUpdatesSbw -> setValue(StatsPluginSettings::updateChartsEveryGuiUpdates());
	pmUi -> DataIvalSbw -> setValue(StatsPluginSettings::gatherDataEveryMs());
	pmUi -> PeersSpdUpdIvalSbw -> setValue(StatsPluginSettings::peersSpeedDataIval());
	
	pmUi -> PeersSpdCbw -> setChecked(StatsPluginSettings::peersSpeed());
	pmUi -> ConnSdrInSwaCbw -> setChecked(StatsPluginSettings::drawSeedersInSwarms());
	pmUi -> ConnLchInSwaCbw -> setChecked(StatsPluginSettings::drawLeechersInSwarms());
	
	pmUi -> DownloadMrmtSbw -> setValue(StatsPluginSettings::downloadMeasurements());
	pmUi -> PeersSpdMrmtSbw -> setValue(StatsPluginSettings::peersSpeedMeasurements());
	pmUi -> UploadMrmtSbw -> setValue(StatsPluginSettings::uploadMeasurements());
	pmUi -> ConnsMrmtSbw -> setValue(StatsPluginSettings::connectionsMeasurements());
	pmUi -> DHTMrmtSbw -> setValue(StatsPluginSettings::dHTMeasurements());
	pmUi -> MaxSpdModeCbw -> setCurrentItem(StatsPluginSettings::maxSpdMode());
}

void StatsPluginPrefs::deleteWidget ()
{
	delete pmUi;
}


} //NS end

#include "StatsPluginPrefs.moc"
