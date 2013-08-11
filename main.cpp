#include <cstdint>
#include <iostream>
#include <list>
#include <cmath>

struct Vec3 {
  float x, y, z;
};

float dot(const Vec3& v1, const Vec3& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3 operator-(const Vec3& v1, const Vec3& v2) {
  return Vec3 { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

struct Ray {
  Vec3 origin;
  Vec3 direction;
};

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct Sphere {
  Vec3 center;
  float radius;
};

bool trace_ray(const Ray& ray, const Sphere& sphere, uint8_t color[3]) {

  const float a = dot(ray.direction, ray.direction);
  const Vec3 c2o = ray.origin - sphere.center;
  const float b = 2 * dot(c2o, ray.direction);
  const float c = dot(c2o, c2o) - sphere.radius * sphere.radius;

  const float delta = b * b - 4 * a * c;

  if (delta < 0) {
    return false;
  }

  const float delta_root = std::sqrt(delta);

  const float roots[2] = {
    (-b - delta_root) / (2 * a),
    (-b + delta_root) / (2 * a),
  };

  float root;

  if (roots[0] < 0 && roots[1] < 0) {
    return false;
  }

  if (roots[0] < 0 || roots[1] < 0) {
    root = std::max(roots[0], roots[1]);
  } else {
    root = std::min(roots[0], roots[1]);
  }

  if (color) {
    // TODO shoot rays, ambient occ style
  }

  return true;

}

bool trace_ray(const Ray& ray, const std::list<Sphere>& world, uint8_t color[3]) {

  for (const Sphere& sphere: world) {
    if (trace_ray(ray, sphere, color)) {
      return true;
    }
  }

  return false;

}

int main() {

  std::list<Sphere> world = {
    { { 0, 0, 0 }, 1 },
  };

  Ray ray;
  ray.direction = Vec3 { 0, 0, 1 };

  const int width = 1920, height = 1080;
  const float zoom = 2;
  std::cout << "P1" << std::endl << width << ' ' << height << std::endl;
  for (float y = 0; y < height; ++y) {
    for (float x = 0; x < width; ++x) {
      ray.origin = Vec3 {
        zoom * (x / width * 2 - 1),
        zoom * height / width * (y / height * 2 - 1),
        -1
      };
      std::cout << !trace_ray(ray, world, nullptr) << ' ';
    }
    std::cout << std::endl;
  }

}
