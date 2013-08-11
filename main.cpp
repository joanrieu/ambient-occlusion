#include <cmath>
#include <cstdint>
#include <iostream>
#include <list>
#include <random>

struct Vec3 {
  float x, y, z;
};

float dot(const Vec3& v1, const Vec3& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3 cross(const Vec3& v1, const Vec3& v2) {
  return Vec3 {
    v1.y * v2.z - v1.z * v2.y,
    v2.x * v1.z - v2.z * v1.x,
    v1.x * v2.y - v1.y * v2.x,
  };
}

Vec3 operator+(const Vec3& v1, const Vec3& v2) {
  return Vec3 { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}
Vec3 operator-(const Vec3& v1, const Vec3& v2) {
  return Vec3 { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

Vec3 operator*(const float& a, const Vec3& v) {
  return Vec3 { a * v.x, a * v.y, a * v.z };
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

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0, M_PI);

bool trace_ray(const Ray& ray, const std::list<Sphere>& world, Color* color);

bool trace_ray(const Ray& ray, const std::list<Sphere>& world, const Sphere& sphere, Color* color) {

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

  if (roots[0] < 0 && roots[1] < 0) {
    return false;
  } else if (!color) {
    return true;
  }

  float root;

  if (roots[0] < 0 || roots[1] < 0) {
    root = std::max(roots[0], roots[1]);
  } else {
    root = std::min(roots[0], roots[1]);
  }

  const Vec3 intersection = ray.origin + root * ray.direction;
  const Vec3 normal = intersection - sphere.center;
  const Vec3 tan1 = (normal.y != 0 || normal.z != 0) ? cross(normal, Vec3 { 1, 0, 0 }) : Vec3 { 0, 1, 0 };
  const Vec3 tan2 = cross(normal, tan1);

  const unsigned tests = 100;
  float occlusion = 0;

  #pragma omp parallel for
  for (int i = 0; i < tests; ++i) {

    const float inclination = dis(gen) / 2;
    const float azimuth = 2 * dis(gen);

    const Vec3 spherical {
      std::cos(azimuth) * std::sin(inclination),
      std::sin(azimuth) * std::sin(inclination),
      std::cos(inclination),
    };

    const Vec3 direction = spherical.x * tan1 + spherical.y * tan2 + spherical.z * normal;

    const Ray test_ray {
      intersection + 0.001f * direction,
      direction,
    };

    if (trace_ray(test_ray, world, nullptr)) {
      occlusion += 1.f / tests;
    }

  }

  color->r = color->g = color->b = 255 * (1 - occlusion) * (1 - occlusion);

  return true;

}

bool trace_ray(const Ray& ray, const std::list<Sphere>& world, Color* color) {

  for (const Sphere& sphere: world) {
    if (trace_ray(ray, world, sphere, color)) {
      return true;
    }
  }

  return false;

}

int main() {

  std::list<Sphere> world = { // front to back
    { { 1, 1, 1 }, .3 },
    { { -1, .5, 2 }, .4 },
    { { 0, 0, 0 }, 1 },
  };

  Ray ray;
  ray.direction = Vec3 { 0, 0, -1 };
  const float zoom = 2;

  const int width = 1920, height = 1080, oversampling = 4;
  const Color background { 0, 0, 0 };

  std::cout << "P3" << std::endl << width << ' ' << height << ' ' << 255 << std::endl;

  for (float y = 0; y < height; ++y) {

    std::cerr << '\r' << int(y * 100 / height) << " %";

    for (float x = 0; x < width; ++x) {

      Color color = background;

      for (int sy = 0; sy < oversampling; ++sy) {
        for (int sx = 0; sx < oversampling; ++sx) {

          ray.origin = Vec3 {
            zoom * ((x * oversampling + sx) / (width * oversampling) * 2 - 1),
            zoom * height / width * -((y * oversampling + sy) / (height * oversampling) * 2 - 1),
            10
          };

          Color sample = background;

          trace_ray(ray, world, &sample);

          color.r += sample.r / (oversampling * oversampling);
          color.g += sample.g / (oversampling * oversampling);
          color.b += sample.b / (oversampling * oversampling);

        }
      }

      std::cout << (int)color.r << ' ' << (int)color.g << ' ' << (int)color.b << ' ';

    }

    std::cout << std::endl;

  }

}
