// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "components/update_client/task_update.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/update_client/update_client.h"
#include "components/update_client/update_engine.h"

namespace update_client {

TaskUpdate::TaskUpdate(UpdateEngine* update_engine,
                       bool is_foreground,
                       const std::vector<std::string>& ids,
                       const UpdateClient::CrxDataCallback& crx_data_callback,
                       const Callback& callback)
    : update_engine_(update_engine),
      is_foreground_(is_foreground),
      ids_(ids),
      crx_data_callback_(crx_data_callback),
      callback_(callback) {
}

TaskUpdate::~TaskUpdate() {
  DCHECK(thread_checker_.CalledOnValidThread());
}

void TaskUpdate::Run() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (ids_.empty()) {
    TaskComplete(Error::ERROR_UPDATE_INVALID_ARGUMENT);
    return;
  }

  update_engine_->Update(
      is_foreground_, ids_, crx_data_callback_,
      base::Bind(&TaskUpdate::TaskComplete, base::Unretained(this)));
}

void TaskUpdate::Cancel() {
  DCHECK(thread_checker_.CalledOnValidThread());

  TaskComplete(Error::ERROR_UPDATE_CANCELED);
}

std::vector<std::string> TaskUpdate::GetIds() const {
  return ids_;
}

void TaskUpdate::TaskComplete(int error) {
  DCHECK(thread_checker_.CalledOnValidThread());

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(callback_, this, error));
}

}  // namespace update_client