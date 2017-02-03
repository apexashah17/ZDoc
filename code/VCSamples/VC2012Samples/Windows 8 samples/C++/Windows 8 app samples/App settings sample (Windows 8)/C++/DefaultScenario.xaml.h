﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// DefaultScenario.xaml.h
// Declaration of the DefaultScenario class
//

#pragma once

#include "pch.h"
#include "DefaultScenario.g.h"
#include "MainPage.xaml.h"

namespace SDKSample
{
namespace ApplicationSettings
{
/// <summary>
/// An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class DefaultScenario sealed
{
public:
    DefaultScenario();

protected:
    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

private:
    MainPage^ rootPage;
};
}
}