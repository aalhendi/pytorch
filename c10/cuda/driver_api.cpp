#if !defined(USE_ROCM) && defined(PYTORCH_C10_DRIVER_API_SUPPORTED)
#include <c10/cuda/driver_api.h>
#include <c10/util/CallOnce.h>
#include <c10/util/Exception.h>
#include <dlfcn.h>
#include <iostream>
namespace c10 {
namespace cuda {

namespace {

DriverAPI create_driver_api() {
  void* handle_0 = dlopen("libcuda.so", RTLD_LAZY | RTLD_NOLOAD);
  TORCH_INTERNAL_ASSERT(handle_0);
  void* handle_1 = DriverAPI::get_nvml_handle();
  DriverAPI r{};

#define LOOKUP_LIBCUDA_ENTRY(name)                       \
  r.name##_ = ((decltype(&name))dlsym(handle_0, #name)); \
  TORCH_INTERNAL_ASSERT(r.name##_)
  C10_LIBCUDA_DRIVER_API(LOOKUP_LIBCUDA_ENTRY)
#undef LOOKUP_LIBCUDA_ENTRY

  if (handle_1) {
#define LOOKUP_NVML_ENTRY(name)                          \
  r.name##_ = ((decltype(&name))dlsym(handle_1, #name)); \
  TORCH_INTERNAL_ASSERT(r.name##_)
    C10_NVML_DRIVER_API(LOOKUP_NVML_ENTRY)
#undef LOOKUP_NVML_ENTRY
  }
  return r;
}
} // namespace

void* DriverAPI::get_nvml_handle() {
  static c10::once_flag once;
  static void* handle_1;
  c10::call_once(
      once, [] { handle_1 = dlopen("libnvidia-ml.so.1", RTLD_LAZY); });
  return handle_1;
}

DriverAPI* DriverAPI::get() {
  static DriverAPI singleton = create_driver_api();
  return &singleton;
}

} // namespace cuda
} // namespace c10

#endif
