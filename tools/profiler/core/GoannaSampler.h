/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GoannaSampler_h
#define GoannaSampler_h

#include "platform.h"
#include "ProfileEntry.h"
#include "mozilla/UniquePtr.h"
#include "mozilla/Vector.h"
#include "ThreadProfile.h"
#include "ThreadInfo.h"
#ifdef MOZ_TASK_TRACER
#include "GoannaTaskTracer.h"
#endif

#include <algorithm>

namespace mozilla {
class ProfileGatherer;
} // namespace mozilla

typedef mozilla::Vector<std::string> ThreadNameFilterList;
typedef mozilla::Vector<std::string> FeatureList;

static bool
threadSelected(ThreadInfo* aInfo, const ThreadNameFilterList &aThreadNameFilters) {
  if (aThreadNameFilters.empty()) {
    return true;
  }

  std::string name = aInfo->Name();
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  for (uint32_t i = 0; i < aThreadNameFilters.length(); ++i) {
    std::string filter = aThreadNameFilters[i];
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    // Crude, non UTF-8 compatible, case insensitive substring search
    if (name.find(filter) != std::string::npos) {
      return true;
    }
  }

  return false;
}

extern mozilla::TimeStamp sLastTracerEvent;
extern int sFrameNumber;
extern int sLastFrameNumber;

class GoannaSampler: public Sampler {
 public:
  GoannaSampler(double aInterval, int aEntrySize,
              const char** aFeatures, uint32_t aFeatureCount,
              const char** aThreadNameFilters, uint32_t aFilterCount);
  ~GoannaSampler();

  void RegisterThread(ThreadInfo* aInfo) {
    if (!aInfo->IsMainThread() && !mProfileThreads) {
      return;
    }

    if (!threadSelected(aInfo, mThreadNameFilters)) {
      return;
    }

    aInfo->SetProfile(mozilla::MakeUnique<ThreadProfile>(aInfo, mBuffer));
  }

  // Called within a signal. This function must be reentrant
  virtual void Tick(TickSample* sample) override;

  // Immediately captures the calling thread's call stack and returns it.
  virtual SyncProfile* GetBacktrace() override;

  // Called within a signal. This function must be reentrant
  virtual void RequestSave() override
  {
    mSaveRequested = true;
  }

  virtual void HandleSaveRequest() override;
  virtual void DeleteExpiredMarkers() override;

  void ToStreamAsJSON(std::ostream& stream, double aSinceTime = 0);
  virtual JSObject *ToJSObject(JSContext *aCx, double aSinceTime = 0);
  void GetGatherer(nsISupports** aRetVal);
  mozilla::UniquePtr<char[]> ToJSON(double aSinceTime = 0);
  virtual void ToJSObjectAsync(double aSinceTime = 0, mozilla::dom::Promise* aPromise = 0);
  void ToFileAsync(const nsACString& aFileName, double aSinceTime = 0);
  void StreamMetaJSCustomObject(SpliceableJSONWriter& aWriter);
  void StreamTaskTracer(SpliceableJSONWriter& aWriter);
  void FlushOnJSShutdown(JSContext* aContext);
  bool ProfileJS() const { return mProfileJS; }
  bool ProfileJava() const { return mProfileJava; }
  bool ProfileGPU() const { return mProfileGPU; }
  bool ProfileThreads() const override { return mProfileThreads; }
  bool InPrivacyMode() const { return mPrivacyMode; }
  bool AddMainThreadIO() const { return mAddMainThreadIO; }
  bool ProfileMemory() const { return mProfileMemory; }
  bool TaskTracer() const { return mTaskTracer; }
  bool LayersDump() const { return mLayersDump; }
  bool DisplayListDump() const { return mDisplayListDump; }
  bool ProfileRestyle() const { return mProfileRestyle; }
  const ThreadNameFilterList& ThreadNameFilters() { return mThreadNameFilters; }
  const FeatureList& Features() { return mFeatures; }

  void GetBufferInfo(uint32_t *aCurrentPosition, uint32_t *aTotalSize, uint32_t *aGeneration);

protected:
  // Called within a signal. This function must be reentrant
  virtual void InplaceTick(TickSample* sample);

  // Not implemented on platforms which do not support backtracing
  void doNativeBacktrace(ThreadProfile &aProfile, TickSample* aSample);

  void StreamJSON(SpliceableJSONWriter& aWriter, double aSinceTime);

  RefPtr<ProfileBuffer> mBuffer;
  bool mSaveRequested;
  bool mAddLeafAddresses;
  bool mUseStackWalk;
  bool mProfileJS;
  bool mProfileGPU;
  bool mProfileThreads;
  bool mProfileJava;
  bool mLayersDump;
  bool mDisplayListDump;
  bool mProfileRestyle;

  // Keep the thread filter to check against new thread that
  // are started while profiling
  ThreadNameFilterList mThreadNameFilters;
  FeatureList mFeatures;
  bool mPrivacyMode;
  bool mAddMainThreadIO;
  bool mProfileMemory;
  bool mTaskTracer;

private:
  RefPtr<mozilla::ProfileGatherer> mGatherer;
};

#endif

