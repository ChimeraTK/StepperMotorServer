// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "StepperMotorServer.h"

#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>
#include <ChimeraTK/ControlSystemAdapter/ApplicationFactory.h>

/// The Server instance
static ChimeraTK::ApplicationFactory<StepperMotorServer> serverFactory;
