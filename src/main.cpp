#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <thread>
#include <vector>
#include "camera.hpp"
#include "float.h"
#include "hitable_list.hpp"
#include "material.hpp"
#include "sphere.hpp"

vec3 color(const ray &r, hitable *world, int depth) {
  hit_record rec;
  if (world->hit(r, 0.001, FLT_MAX, rec)) {
    ray scattered;
    vec3 attenuation;
    if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
      return attenuation * color(scattered, world, depth + 1);
    } else {
      return vec3(0, 0, 0);
    }
  } else {
    vec3 unit_direction = unit_vector(r.direction());
    float t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  }
}

std::vector<vec3> render_section(int nx, int ny, int top, int bottom, int ns,
                                 const camera &cam, hitable *world) {
  std::vector<vec3> pixels;
  pixels.reserve(nx * ny);
  // std::cout << "Rendering: " << top - 1 << " : " << bottom << "\n";
  for (int j = top - 1; j >= bottom; j--) {
    for (int i = 0; i < nx; i++) {
      vec3 col(0, 0, 0);
      for (int s = 0; s < ns; s++) {
        float u = float(i + drand48()) / float(nx);
        float v = float(j + drand48()) / float(ny);
        ray r = cam.get_ray(u, v);
        vec3 p = r.point_at_parameter(2.0);
        col += color(r, world, 0);
      }
      col /= float(ns);

      // gamma correction
      col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
      int ir = int(245.99 * col[0]);
      int ig = int(255.99 * col[1]);
      int ib = int(255.99 * col[2]);

      pixels.push_back(vec3(ir, ig, ib));
    }
  }
  return pixels;
}

int main() {
  int threads = 12;
  int factor = 2;
  int x = 200 * factor;
  int y = 100 * factor;
  int s = 100;

  std::ofstream file;
  file.open("./bin/picture.ppm", std::ios::out | std::ios::trunc);

  file << "P3\n" << x << " " << y << "\n255\n";

  int number_of_obj = 5;
  hitable *list[number_of_obj];
  list[0] =
      new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
  list[1] =
      new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
  list[2] =
      new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.1));
  list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
  list[4] = new sphere(vec3(-1, 0, -1), -0.49, new dielectric(1.5));

  hitable *world = new hitable_list(list, number_of_obj);
  camera cam(vec3(-1, 2, 1), vec3(0, 0, -1), vec3(0, 1, 0), 20,
             float(x) / float(y));

  auto start = std::chrono::high_resolution_clock::now();

  std::vector<std::future<std::vector<vec3>>> futures;

  int jump = y / threads;
  if (y % threads != 0) threads++;

  int top = y;
  int bottom = top - jump;
  for (int t = 1; t <= threads; t++) {
    futures.push_back(
        std::async(render_section, x, y, top, bottom, s, cam, world));
    top = bottom;
    bottom -= jump;
    bottom = (bottom < 0) ? 0 : bottom;
  }

  for (auto &f : futures) {
    std::vector<vec3> pixels = f.get();
    for (auto &col : pixels) {
      file << col[0] << " " << col[1] << " " << col[2] << "\n";
    }
  }

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count() << " s\n";

  file.close();
}
