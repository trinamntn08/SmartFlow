# Isolated local changes

- `tp_math_utils/src/Intersection.cpp`: replace the incorrect projected-point
  formula with the standard viewing-line/plane intersection. Use double
  intermediates; reject parallel, coplanar, degenerate and non-finite inputs.
  Return false without a usable intersection. Signed line intersections are
  intentional for orthographic projection. Regression coverage is in
  `native/tests/SceneControllerTests.cpp`. All other copied legacy files remain
  byte-identical to the manifest.
- `adapted/LegacyNavigation.h`: new attributed adaptation of CADController's
  aspect-normalized orthographic pan; takes full viewport span instead of the
  legacy half-span distance. No Qt map/rendering dependencies.
