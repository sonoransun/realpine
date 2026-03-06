// docker-bake.hcl — Multi-architecture build definitions
//
// Usage:
//   docker buildx bake -f docker/docker-bake.hcl
//   docker buildx bake -f docker/docker-bake.hcl alpine   # lightweight image

variable "REGISTRY" {
  default = ""
}

variable "TAG" {
  default = "latest"
}

group "default" {
  targets = ["alpine-node"]
}

group "all" {
  targets = ["alpine-node", "alpine"]
}

target "alpine-node" {
  context    = ".."
  dockerfile = "docker/Dockerfile"
  platforms  = ["linux/amd64", "linux/arm64"]
  tags       = [
    "${REGISTRY}alpine-node:${TAG}",
  ]
}

target "alpine" {
  context    = ".."
  dockerfile = "docker/Dockerfile.alpine"
  platforms  = ["linux/amd64", "linux/arm64"]
  tags       = [
    "${REGISTRY}alpine-node:${TAG}-alpine",
  ]
}
