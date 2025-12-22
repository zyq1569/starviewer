/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include "extensioncontext.h"

#include "patient.h"
#include "volume.h"
#include "logging.h"

namespace udg {

ExtensionContext::ExtensionContext()
{
}

ExtensionContext::~ExtensionContext()
{
}

Patient* ExtensionContext::getPatient() const
{
    return m_patient;
}

void ExtensionContext::setPatient(Patient *patient)
{
    m_patient = patient;
}

Volume *ExtensionContext::getDefaultVolume() const
{
    Volume *defaultVolume = NULL;
    Series *defaultSeries = NULL;
    bool searchForDefaultSeries = false;
    QList<Series*> selectedSeries = m_patient->getSelectedSeries();

    if (selectedSeries.isEmpty())
    {
        searchForDefaultSeries = true;
    }
    else
    {
        // EVERYTHING at the moment we only take the first of the possible ones selected
        defaultSeries = selectedSeries.at(0);
        // We need the series to be viewable
		defaultSeries->isCTLocalizer();
        if (!defaultSeries->isViewable())
        {
            searchForDefaultSeries = true;
        }
        else
        {
            defaultVolume = defaultSeries->getFirstVolume();
        }
    }
    // Instead of searchForDefaultSeries we could use
    // defaultVolume, but with var. boolean code is more readable
    if (searchForDefaultSeries)
    {
        bool ok = false;
        foreach (Study *study, m_patient->getStudies())
        {
            QList<Series*> viewableSeries = study->getViewableSeries();
            if (!viewableSeries.isEmpty())
            {
                ok = true;
                defaultVolume = viewableSeries.at(0)->getFirstVolume();
                break;
            }
        }
        if (!ok)
        {
            DEBUG_LOG("There is no current patient series that is viewable. Return volume NULL.");
            ERROR_LOG("There is no current patient series that is viewable. Return volume NULL.");
        }
    }

    return defaultVolume;
}

Volume *ExtensionContext::getDefaultVolumeNoLocalizer() const
{
	Volume *defaultVolume = NULL;
	Series *defaultSeries = NULL;
	bool searchForDefaultSeries = false;
	QList<Series*> selectedSeries = m_patient->getSelectedSeries();

	if (selectedSeries.isEmpty())
	{
		searchForDefaultSeries = true;
	}
	else
	{
		// EVERYTHING at the moment we only take the first of the possible ones selected
		defaultSeries = selectedSeries.at(0);
		// We need the series to be viewable
		if (!defaultSeries->isViewable() || defaultSeries->isCTLocalizer())
		{
			searchForDefaultSeries = true;
		}
		else
		{
			defaultVolume = defaultSeries->getFirstVolume();
		}
	}
	// Instead of searchForDefaultSeries we could use
	// defaultVolume, but with var. boolean code is more readable
	if (searchForDefaultSeries)
	{
		bool ok = false;
		foreach(Study *study, m_patient->getStudies())
		{
			QList<Series*> viewableSeries = study->getViewableSeries();
			if (!viewableSeries.isEmpty())
			{
				foreach (Series*  series , viewableSeries)
				{
					if (!series->isCTLocalizer())
					{
						defaultVolume = series->getFirstVolume();
						return defaultVolume;
					}
				}
			}
		}
		if (!ok)
		{
			DEBUG_LOG("There is no current patient series that is viewable. Return volume NULL.");
			ERROR_LOG("There is no current patient series that is viewable. Return volume NULL.");
		}
	}

	return defaultVolume;
}
bool ExtensionContext::hasImages() const
{
    if (!m_patient)
    {
        return false;
    }

    foreach (Study *study, m_patient->getStudies())
    {
        foreach (Series *series, study->getSeries())
        {
            if (series->hasImages())
            {
                return true;
            }
        }
    }

    return false;
}

bool ExtensionContext::hasEncapsulatedDocuments() const
{
    if (!m_patient)
    {
        return false;
    }

    foreach (Study *study, m_patient->getStudies())
    {
        foreach (Series *series, study->getSeries())
        {
            if (series->hasEncapsulatedDocuments())
            {
                return true;
            }
        }
    }

    return false;
}

}
