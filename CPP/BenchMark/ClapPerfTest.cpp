// Standalone, deterministic before/after timing harness for the
// IntersectListSort optimization in clipper.engine.cpp.
//
// Exercises ClipperBase::Execute() via the public BooleanOp(Intersection)
// API on self-intersecting random polygons, which drives
// ProcessIntersectList()/IntersectListSort() heavily (per the optimization
// commit's own profiling notes). Uses a fixed PRNG seed so the workload is
// identical across "before" and "after" builds.
//
// Prints exactly one line: PERF_TIME_SEC=<float seconds>

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <random>

#include "clipper2/clipper.h"

using namespace Clipper2Lib;

namespace {

Path64 MakeRandomPoly(std::mt19937& rng, int width, int height, unsigned vertCnt)
{
  std::uniform_int_distribution<int> distX(0, width - 1);
  std::uniform_int_distribution<int> distY(0, height - 1);
  Path64 result;
  result.reserve(vertCnt);
  for (unsigned i = 0; i < vertCnt; ++i)
    result.push_back(Point64(distX(rng), distY(rng)));
  return result;
}

} // namespace

int main()
{
  const int width = 800, height = 600;
  const unsigned vertCnt = 500;
  const int iterations = 200;

  std::mt19937 rng(12345);
  ClipType ct = ClipType::Intersection;
  FillRule fr = FillRule::NonZero;

  uint64_t total_points = 0;

  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < iterations; ++i)
  {
    Paths64 subject, clip, solution;
    subject.push_back(MakeRandomPoly(rng, width, height, vertCnt));
    clip.push_back(MakeRandomPoly(rng, width, height, vertCnt));

    solution = BooleanOp(ct, fr, subject, clip);

    for (const auto& path : solution)
      total_points += path.size();
  }
  auto t1 = std::chrono::steady_clock::now();

  double secs = std::chrono::duration<double>(t1 - t0).count();

  // Prevent the compiler from optimizing away the loop and provide a
  // deterministic-output sanity check (printed to stderr, not parsed).
  std::fprintf(stderr, "total_point_count=%llu\n",
    static_cast<unsigned long long>(total_points));
  std::printf("PERF_TIME_SEC=%.6f\n", secs);
  return 0;
}
