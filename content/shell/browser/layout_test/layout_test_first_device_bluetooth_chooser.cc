// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/layout_test/layout_test_first_device_bluetooth_chooser.h"

#include "base/logging.h"

namespace content {

LayoutTestFirstDeviceBluetoothChooser::LayoutTestFirstDeviceBluetoothChooser(
    const EventHandler& event_handler)
    : event_handler_(event_handler) {}

LayoutTestFirstDeviceBluetoothChooser::
    ~LayoutTestFirstDeviceBluetoothChooser() {}

void LayoutTestFirstDeviceBluetoothChooser::SetAdapterPresence(
    AdapterPresence presence) {
  switch (presence) {
    case AdapterPresence::ABSENT:
    case AdapterPresence::POWERED_OFF:
      // Without a user-visible dialog, if the adapter is off, there's no way to
      // ask the user to turn it on again, so we should cancel.
      event_handler_.Run(Event::CANCELLED, "");
      break;
    case AdapterPresence::POWERED_ON:
      break;
  }
}

void LayoutTestFirstDeviceBluetoothChooser::ShowDiscoveryState(
    DiscoveryState state) {
  switch (state) {
    case DiscoveryState::FAILED_TO_START:
    case DiscoveryState::IDLE:
      // Without a user-visible dialog, if discovery finishes without finding a
      // device, we'll never find one, so we should cancel.
      VLOG(1) << "LayoutTestFirstDeviceBluetoothChooser found nothing before "
                 "going idle.";
      event_handler_.Run(Event::CANCELLED, "");
      break;
    case DiscoveryState::DISCOVERING:
      break;
  }
}

void LayoutTestFirstDeviceBluetoothChooser::AddDevice(
    const std::string& deviceId,
    const base::string16& deviceName) {
  event_handler_.Run(Event::SELECTED, deviceId);
}

}  // namespace content
