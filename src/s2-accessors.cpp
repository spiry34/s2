
#include "geography-operator.h"
#include "s2/s2closest_edge_query.h"
#include "s2/s2furthest_edge_query.h"
#include <Rcpp.h>
using namespace Rcpp;

#include "s2-geography/accessors.hpp"

// [[Rcpp::export]]
LogicalVector cpp_s2_is_collection(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_is_collection(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_valid(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      return !(feature->FindValidationError(&(this->error)));
    }

    S2Error error;
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
CharacterVector cpp_s2_is_valid_reason(List geog) {
  class Op: public UnaryGeographyOperator<CharacterVector, String> {
    String processFeature(XPtr<Geography> feature, R_xlen_t i) {
      if (feature->FindValidationError(&(this->error))) {
        return this->error.text();
      } else {
        return NA_STRING;
      }
    }

    S2Error error;
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_dimension(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_dimension(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
IntegerVector cpp_s2_num_points(List geog) {
  class Op: public UnaryGeographyOperator<IntegerVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_num_points(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
LogicalVector cpp_s2_is_empty(List geog) {
  class Op: public UnaryGeographyOperator<LogicalVector, int> {
    int processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_is_empty(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_area(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_area(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_length(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_length(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_perimeter(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      return s2geography::s2_perimeter(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_x(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      if (s2geography::s2_dimension(*geog) != 0) {
        Rcpp::stop("Can't compute X value of a non-point geography");
      }

      return s2geography::s2_x(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_y(List geog) {
  class Op: public UnaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature, R_xlen_t i) {
      auto geog = feature->NewGeography();
      if (s2geography::s2_dimension(*geog) != 0) {
        Rcpp::stop("Can't compute Y value of a non-point geography");
      }

      return s2geography::s2_y(*geog);
    }
  };

  Op op;
  return op.processVector(geog);
}

// [[Rcpp::export]]
NumericVector cpp_s2_project_normalized(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {
    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      if (feature1->IsCollection() || feature2->IsCollection()) {
        throw GeographyOperatorException("`x` and `y` must both be simple geographies");
      }

      if (feature1->IsEmpty() || feature2->IsEmpty()) {
        return NA_REAL;
      }

      if (feature1->GeographyType() == Geography::Type::GEOGRAPHY_POLYLINE) {
        if (feature2->GeographyType() == Geography::Type::GEOGRAPHY_POINT) {
          S2Point point = feature2->Point()->at(0);
          int next_vertex;
          S2Point point_on_line = feature1->Polyline()->at(0)->Project(point, &next_vertex);
          return feature1->Polyline()->at(0)->UnInterpolate(point_on_line, next_vertex);
        } else {
          throw GeographyOperatorException("`y` must be a point geography");
        }
      } else {
        throw GeographyOperatorException("`x` must be a polyline geography");
      }
      return NA_REAL;
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      S2ClosestEdgeQuery query(feature1->ShapeIndex());
      S2ClosestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindClosestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      if (distance == R_PosInf) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}

// [[Rcpp::export]]
NumericVector cpp_s2_max_distance(List geog1, List geog2) {
  class Op: public BinaryGeographyOperator<NumericVector, double> {

    double processFeature(XPtr<Geography> feature1,
                          XPtr<Geography> feature2,
                          R_xlen_t i) {
      S2FurthestEdgeQuery query(feature1->ShapeIndex());
      S2FurthestEdgeQuery::ShapeIndexTarget target(feature2->ShapeIndex());

      const auto& result = query.FindFurthestEdge(&target);

      S1ChordAngle angle = result.distance();
      double distance = angle.ToAngle().radians();

      // returns -1 if one of the indexes is empty
      // NA is more consistent with the BigQuery
      // function, and makes way more sense
      if (distance < 0) {
        return NA_REAL;
      } else {
        return distance;
      }
    }
  };

  Op op;
  return op.processVector(geog1, geog2);
}
