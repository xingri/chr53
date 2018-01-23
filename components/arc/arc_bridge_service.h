// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ARC_ARC_BRIDGE_SERVICE_H_
#define COMPONENTS_ARC_ARC_BRIDGE_SERVICE_H_

#include <string>
#include <vector>

#include "base/files/scoped_file.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "base/values.h"
#include "components/arc/common/arc_bridge.mojom.h"

namespace base {
class CommandLine;
}  // namespace base

namespace arc {

// The Chrome-side service that handles ARC instances and ARC bridge creation.
// This service handles the lifetime of ARC instances and sets up the
// communication channel (the ARC bridge) used to send and receive messages.
class ArcBridgeService : public mojom::ArcBridgeHost {
 public:
  // The possible states of the bridge.  In the normal flow, the state changes
  // in the following sequence:
  //
  // STOPPED
  //   PrerequisitesChanged() ->
  // CONNECTING
  //   OnConnectionEstablished() ->
  // READY
  //
  // The ArcBridgeBootstrap state machine can be thought of being substates of
  // ArcBridgeService's CONNECTING state.
  //
  // *
  //   StopInstance() ->
  // STOPPING
  //   OnStopped() ->
  // STOPPED
  enum class State {
    // ARC is not currently running.
    STOPPED,

    // The request to connect has been sent.
    CONNECTING,

    // The instance has started, and the bridge is fully established.
    CONNECTED,

    // The ARC instance has finished initializing and is now ready for user
    // interaction.
    READY,

    // The ARC instance has started shutting down.
    STOPPING,
  };

  // Describes the reason the bridge is stopped.
  enum class StopReason {
    // ARC instance has been gracefully shut down.
    SHUTDOWN,

    // Errors occurred during the ARC instance boot. This includes any failures
    // before the instance is actually attempted to be started, and also
    // failures on bootstrapping IPC channels with Android.
    GENERIC_BOOT_FAILURE,

    // The device is critically low on disk space.
    LOW_DISK_SPACE,

    // ARC instance has crashed.
    CRASH,
  };

  // Notifies life cycle events of ArcBridgeService.
  class Observer {
   public:
    // Called whenever the state of the bridge has changed.
    // TODO(lchavez): Rename to OnStateChangedForTest
    virtual void OnStateChanged(State state) {}
    virtual void OnBridgeReady() {}
    virtual void OnBridgeStopped(StopReason reason) {}

    // Called whenever ARC's availability has changed for this system.
    virtual void OnAvailableChanged(bool available) {}

    // Called whenever the ARC app interface state changes.
    virtual void OnAppInstanceReady() {}
    virtual void OnAppInstanceClosed() {}

    // Called whenever the ARC audio interface state changes.
    virtual void OnAudioInstanceReady() {}
    virtual void OnAudioInstanceClosed() {}

    // Called whenever the ARC auth interface state changes.
    virtual void OnAuthInstanceReady() {}
    virtual void OnAuthInstanceClosed() {}

    // Called whenever ARC Bluetooth instance is ready.
    virtual void OnBluetoothInstanceReady() {}
    virtual void OnBluetoothInstanceClosed() {}

    // Called whenever the ARC clipboard interface state changes.
    virtual void OnClipboardInstanceReady() {}
    virtual void OnClipboardInstanceClosed() {}

    // Called whenever the ARC crash collector interface state changes.
    virtual void OnCrashCollectorInstanceReady() {}
    virtual void OnCrashCollectorInstanceClosed() {}

    // Called whenever the ARC file system interface state changes.
    virtual void OnFileSystemInstanceReady() {}
    virtual void OnFileSystemInstanceClosed() {}

    // Called whenever the ARC IME interface state changes.
    virtual void OnImeInstanceReady() {}
    virtual void OnImeInstanceClosed() {}

    // Called whenever the ARC intent helper interface state changes.
    virtual void OnIntentHelperInstanceReady() {}
    virtual void OnIntentHelperInstanceClosed() {}

    // Called whenever the ARC metrics interface state changes.
    virtual void OnMetricsInstanceReady() {}
    virtual void OnMetricsInstanceClosed() {}

    // Called whenever the ARC notification interface state changes.
    virtual void OnNotificationsInstanceReady() {}
    virtual void OnNotificationsInstanceClosed() {}

    // Called whenever the ARC net interface state changes.
    virtual void OnNetInstanceReady() {}
    virtual void OnNetInstanceClosed() {}

    // Called whenever the ARC OBB mounter interface state changes.
    virtual void OnObbMounterInstanceReady() {}
    virtual void OnObbMounterInstanceClosed() {}

    // Called whenever the ARC policy interface state changes.
    virtual void OnPolicyInstanceReady() {}
    virtual void OnPolicyInstanceClosed() {}

    // Called whenever the ARC power interface state changes.
    virtual void OnPowerInstanceReady() {}
    virtual void OnPowerInstanceClosed() {}

    // Called whenever the ARC process interface state changes.
    virtual void OnProcessInstanceReady() {}
    virtual void OnProcessInstanceClosed() {}

    // Called whenever the ARC storage manager interface state changes.
    virtual void OnStorageManagerInstanceReady() {}
    virtual void OnStorageManagerInstanceClosed() {}

    // Called whenever the ARC video interface state changes.
    virtual void OnVideoInstanceReady() {}
    virtual void OnVideoInstanceClosed() {}

    // Called whenever the ARC window manager interface state changes.
    virtual void OnWindowManagerInstanceReady() {}
    virtual void OnWindowManagerInstanceClosed() {}

   protected:
    virtual ~Observer() {}
  };

  ~ArcBridgeService() override;

  // Gets the global instance of the ARC Bridge Service. This can only be
  // called on the thread that this class was created on.
  static ArcBridgeService* Get();

  // Return true if ARC has been enabled through a commandline
  // switch.
  static bool GetEnabled(const base::CommandLine* command_line);

  // SetDetectedAvailability() should be called once CheckArcAvailability() on
  // the session_manager is called. This can only be called on the thread that
  // this class was created on.
  virtual void SetDetectedAvailability(bool availability) = 0;

  // HandleStartup() should be called upon profile startup.  This will only
  // launch an instance if the instance service is available and it is enabled.
  // This can only be called on the thread that this class was created on.
  virtual void HandleStartup() = 0;

  // Shutdown() should be called when the browser is shutting down. This can
  // only be called on the thread that this class was created on.
  virtual void Shutdown() = 0;

  // Adds or removes observers. This can only be called on the thread that this
  // class was created on. RemoveObserver does nothing if |observer| is not in
  // the list.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Gets the Mojo interface for all the instance services. This will return
  // nullptr if that particular service is not ready yet. Use an Observer if
  // you want to be notified when this is ready. This can only be called on the
  // thread that this class was created on.
  mojom::AppInstance* app_instance() { return app_ptr_.get(); }
  mojom::AudioInstance* audio_instance() { return audio_ptr_.get(); }
  mojom::AuthInstance* auth_instance() { return auth_ptr_.get(); }
  mojom::BluetoothInstance* bluetooth_instance() {
    return bluetooth_ptr_.get();
  }
  mojom::ClipboardInstance* clipboard_instance() {
    return clipboard_ptr_.get();
  }
  mojom::CrashCollectorInstance* crash_collector_instance() {
    return crash_collector_ptr_.get();
  }
  mojom::FileSystemInstance* file_system_instance() {
    return file_system_ptr_.get();
  }
  mojom::ImeInstance* ime_instance() { return ime_ptr_.get(); }
  mojom::IntentHelperInstance* intent_helper_instance() {
    return intent_helper_ptr_.get();
  }
  mojom::MetricsInstance* metrics_instance() { return metrics_ptr_.get(); }
  mojom::NetInstance* net_instance() { return net_ptr_.get(); }
  mojom::NotificationsInstance* notifications_instance() {
    return notifications_ptr_.get();
  }
  mojom::ObbMounterInstance* obb_mounter_instance() {
    return obb_mounter_ptr_.get();
  }
  mojom::PolicyInstance* policy_instance() { return policy_ptr_.get(); }
  mojom::PowerInstance* power_instance() { return power_ptr_.get(); }
  mojom::ProcessInstance* process_instance() { return process_ptr_.get(); }
  mojom::StorageManagerInstance* storage_manager_instance() {
    return storage_manager_ptr_.get();
  }
  mojom::VideoInstance* video_instance() { return video_ptr_.get(); }
  mojom::WindowManagerInstance* window_manager_instance() {
    return window_manager_ptr_.get();
  }

  int32_t app_version() const { return app_ptr_.version(); }
  int32_t audio_version() const { return audio_ptr_.version(); }
  int32_t bluetooth_version() const { return bluetooth_ptr_.version(); }
  int32_t auth_version() const { return auth_ptr_.version(); }
  int32_t clipboard_version() const { return clipboard_ptr_.version(); }
  int32_t crash_collector_version() const {
    return crash_collector_ptr_.version();
  }
  int32_t file_system_version() const { return file_system_ptr_.version(); }
  int32_t ime_version() const { return ime_ptr_.version(); }
  int32_t intent_helper_version() const { return intent_helper_ptr_.version(); }
  int32_t metrics_version() const { return metrics_ptr_.version(); }
  int32_t net_version() const { return net_ptr_.version(); }
  int32_t notifications_version() const { return notifications_ptr_.version(); }
  int32_t obb_mounter_version() const { return obb_mounter_ptr_.version(); }
  int32_t policy_version() const { return policy_ptr_.version(); }
  int32_t power_version() const { return power_ptr_.version(); }
  int32_t process_version() const { return process_ptr_.version(); }
  int32_t storage_manager_version() const {
    return storage_manager_ptr_.version();
  }
  int32_t video_version() const { return video_ptr_.version(); }
  int32_t window_manager_version() const {
    return window_manager_ptr_.version();
  }

  // ArcHost:
  void OnAppInstanceReady(mojom::AppInstancePtr app_ptr) override;
  void OnAudioInstanceReady(mojom::AudioInstancePtr audio_ptr) override;
  void OnAuthInstanceReady(mojom::AuthInstancePtr auth_ptr) override;
  void OnBluetoothInstanceReady(
      mojom::BluetoothInstancePtr bluetooth_ptr) override;
  void OnClipboardInstanceReady(
      mojom::ClipboardInstancePtr clipboard_ptr) override;
  void OnCrashCollectorInstanceReady(
      mojom::CrashCollectorInstancePtr crash_collector_ptr) override;
  void OnFileSystemInstanceReady(
      mojom::FileSystemInstancePtr file_system_ptr) override;
  void OnImeInstanceReady(mojom::ImeInstancePtr ime_ptr) override;
  void OnIntentHelperInstanceReady(
      mojom::IntentHelperInstancePtr intent_helper_ptr) override;
  void OnMetricsInstanceReady(mojom::MetricsInstancePtr metrics_ptr) override;
  void OnNetInstanceReady(mojom::NetInstancePtr net_ptr) override;
  void OnNotificationsInstanceReady(
      mojom::NotificationsInstancePtr notifications_ptr) override;
  void OnObbMounterInstanceReady(
      mojom::ObbMounterInstancePtr obb_mounter_ptr) override;
  void OnPolicyInstanceReady(mojom::PolicyInstancePtr policy_ptr) override;
  void OnPowerInstanceReady(mojom::PowerInstancePtr power_ptr) override;
  void OnProcessInstanceReady(mojom::ProcessInstancePtr process_ptr) override;
  void OnStorageManagerInstanceReady(
      mojom::StorageManagerInstancePtr storage_manager_ptr) override;
  void OnVideoInstanceReady(mojom::VideoInstancePtr video_ptr) override;
  void OnWindowManagerInstanceReady(
      mojom::WindowManagerInstancePtr window_manager_ptr) override;

  // Gets the current state of the bridge service.
  State state() const { return state_; }

  // Gets if ARC is available in this system.
  bool available() const { return available_; }

 protected:
  ArcBridgeService();

  // Changes the current state and notifies all observers.
  void SetState(State state);

  // Changes the current availability and notifies all observers.
  void SetAvailable(bool availability);

  // Sets the reason the bridge is stopped. This function must be always called
  // before SetState(State::STOPPED) to report a correct reason with
  // Observer::OnBridgeStopped().
  void SetStopReason(StopReason stop_reason);

  base::ObserverList<Observer>& observer_list() { return observer_list_; }

  bool CalledOnValidThread();

  // Closes all Mojo channels.
  void CloseAllChannels();

 private:
  friend class ArcBridgeTest;
  FRIEND_TEST_ALL_PREFIXES(ArcBridgeTest, Basic);
  FRIEND_TEST_ALL_PREFIXES(ArcBridgeTest, Prerequisites);
  FRIEND_TEST_ALL_PREFIXES(ArcBridgeTest, ShutdownMidStartup);
  FRIEND_TEST_ALL_PREFIXES(ArcBridgeTest, Restart);
  FRIEND_TEST_ALL_PREFIXES(ArcBridgeTest, OnBridgeStopped);

  // Called when one of the individual channels is closed.
  void CloseAppChannel();
  void CloseAudioChannel();
  void CloseAuthChannel();
  void CloseBluetoothChannel();
  void CloseClipboardChannel();
  void CloseCrashCollectorChannel();
  void CloseFileSystemChannel();
  void CloseImeChannel();
  void CloseIntentHelperChannel();
  void CloseMetricsChannel();
  void CloseNetChannel();
  void CloseNotificationsChannel();
  void CloseObbMounterChannel();
  void ClosePolicyChannel();
  void ClosePowerChannel();
  void CloseProcessChannel();
  void CloseStorageManagerChannel();
  void CloseVideoChannel();
  void CloseWindowManagerChannel();

  // Callbacks for QueryVersion.
  void OnAppVersionReady(uint32_t version);
  void OnAudioVersionReady(uint32_t version);
  void OnAuthVersionReady(uint32_t version);
  void OnBluetoothVersionReady(uint32_t version);
  void OnClipboardVersionReady(uint32_t version);
  void OnCrashCollectorVersionReady(uint32_t version);
  void OnFileSystemVersionReady(uint32_t version);
  void OnImeVersionReady(uint32_t version);
  void OnIntentHelperVersionReady(uint32_t version);
  void OnMetricsVersionReady(uint32_t version);
  void OnNetVersionReady(uint32_t version);
  void OnNotificationsVersionReady(uint32_t version);
  void OnObbMounterVersionReady(uint32_t version);
  void OnPolicyVersionReady(uint32_t version);
  void OnPowerVersionReady(uint32_t version);
  void OnProcessVersionReady(uint32_t version);
  void OnStorageManagerVersionReady(uint32_t version);
  void OnVideoVersionReady(uint32_t version);
  void OnWindowManagerVersionReady(uint32_t version);

  // Mojo interfaces.
  mojom::AppInstancePtr app_ptr_;
  mojom::AudioInstancePtr audio_ptr_;
  mojom::AuthInstancePtr auth_ptr_;
  mojom::BluetoothInstancePtr bluetooth_ptr_;
  mojom::ClipboardInstancePtr clipboard_ptr_;
  mojom::CrashCollectorInstancePtr crash_collector_ptr_;
  mojom::FileSystemInstancePtr file_system_ptr_;
  mojom::ImeInstancePtr ime_ptr_;
  mojom::IntentHelperInstancePtr intent_helper_ptr_;
  mojom::MetricsInstancePtr metrics_ptr_;
  mojom::NetInstancePtr net_ptr_;
  mojom::NotificationsInstancePtr notifications_ptr_;
  mojom::ObbMounterInstancePtr obb_mounter_ptr_;
  mojom::PolicyInstancePtr policy_ptr_;
  mojom::PowerInstancePtr power_ptr_;
  mojom::ProcessInstancePtr process_ptr_;
  mojom::StorageManagerInstancePtr storage_manager_ptr_;
  mojom::VideoInstancePtr video_ptr_;
  mojom::WindowManagerInstancePtr window_manager_ptr_;

  // Temporary Mojo interfaces.  After a Mojo interface pointer has been
  // received from the other endpoint, we still need to asynchronously query
  // its version.  While that is going on, we should still return nullptr on
  // the xxx_instance() functions.
  // To keep the xxx_instance() functions being trivial, store the instance
  // pointer in a temporary variable to avoid losing its reference.
  mojom::AppInstancePtr temporary_app_ptr_;
  mojom::AudioInstancePtr temporary_audio_ptr_;
  mojom::AuthInstancePtr temporary_auth_ptr_;
  mojom::BluetoothInstancePtr temporary_bluetooth_ptr_;
  mojom::ClipboardInstancePtr temporary_clipboard_ptr_;
  mojom::CrashCollectorInstancePtr temporary_crash_collector_ptr_;
  mojom::FileSystemInstancePtr temporary_file_system_ptr_;
  mojom::ImeInstancePtr temporary_ime_ptr_;
  mojom::IntentHelperInstancePtr temporary_intent_helper_ptr_;
  mojom::MetricsInstancePtr temporary_metrics_ptr_;
  mojom::NetInstancePtr temporary_net_ptr_;
  mojom::NotificationsInstancePtr temporary_notifications_ptr_;
  mojom::ObbMounterInstancePtr temporary_obb_mounter_ptr_;
  mojom::PolicyInstancePtr temporary_policy_ptr_;
  mojom::PowerInstancePtr temporary_power_ptr_;
  mojom::ProcessInstancePtr temporary_process_ptr_;
  mojom::StorageManagerInstancePtr temporary_storage_manager_ptr_;
  mojom::VideoInstancePtr temporary_video_ptr_;
  mojom::WindowManagerInstancePtr temporary_window_manager_ptr_;

  base::ObserverList<Observer> observer_list_;

  base::ThreadChecker thread_checker_;

  // If the ARC instance service is available.
  bool available_;

  // The current state of the bridge.
  ArcBridgeService::State state_;

  // The reason the bridge is stopped.
  StopReason stop_reason_;

  // WeakPtrFactory to use callbacks.
  base::WeakPtrFactory<ArcBridgeService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ArcBridgeService);
};

}  // namespace arc

#endif  // COMPONENTS_ARC_ARC_BRIDGE_SERVICE_H_
