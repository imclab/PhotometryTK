// __BEGIN_LICENSE__
// Copyright (C) 2006-2010 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__


#ifndef __ASP_PHO_ERROR_ACCUMULATORS_H__
#define __ASP_PHO_ERROR_ACCUMULATORS_H__

#include <asp/PhotometryTK/RecursiveBBoxAccumulator.h>

using namespace vw;

namespace asp {
namespace pho {

  // Calculate error
  class ErrorAccumulator : public vw::ReturnFixedType<void> {
    double m_error_sum, m_curr_time;
  public:
    typedef double value_type;
    ErrorAccumulator(double t) : m_error_sum(0), m_curr_time(t) {}

    template <class AValT, class RValT>
    void operator()( AValT const& drg, AValT const& albedo, RValT const& reflectance ) {
      double i = drg[0], a=albedo[0], s=drg[1], r=reflectance;
      i -= a*m_curr_time*r;
      i *= s; i *= i;
      m_error_sum += i;
    }

    value_type value() const {
      return m_error_sum;
    }
  };

  // Calculate error
  class ErrorNRAccumulator : public vw::ReturnFixedType<void> {
    double m_error_sum, m_curr_time;
  public:
    typedef double value_type;
    ErrorNRAccumulator(double t) : m_error_sum(0), m_curr_time(t) {}

    template <class AValT>
    void operator()( AValT const& drg, AValT const& albedo ) {
      double i = drg[0], a=albedo[0], s=drg[1];
      i -= a*m_curr_time;
      i *= s; i *= i;
      m_error_sum += i;
    }

    value_type value() const {
      return m_error_sum;
    }
  };

  // Wraps error accumulator
  template<typename ScalarT, typename ElementT>
  class ErrorNRAccumulatorFunc : public AccumulatorFunc<ScalarT, ElementT> {
  protected:
    int m_level;
    int32 m_transactionId;
    double m_exposureTime;
    boost::shared_ptr<PlateFile> m_drg;
    boost::shared_ptr<PlateFile> m_albedo;

    ScalarT m_sum;
    
  public:
    ErrorNRAccumulatorFunc(int level,
			   int32 transactionId,
			   double exposureTime,
			   boost::shared_ptr<PlateFile> drg,
			   boost::shared_ptr<PlateFile> albedo) : 
    m_level(level), m_transactionId(transactionId), m_exposureTime(exposureTime), m_drg(drg), m_albedo(albedo), m_sum(0) {}
    
    ErrorNRAccumulatorFunc(ErrorNRAccumulatorFunc const& other) :
    m_level(other.m_level), m_transactionId(other.m_transactionId), m_exposureTime(other.m_exposureTime), m_drg(other.m_drg), m_albedo(other.m_albedo), m_sum(other.m_sum) {}

    ~ErrorNRAccumulatorFunc() {}

    /**
     * For this class, ElementT must be something that holds to int values
     * and has array accessor operators.
     */
    void operator()(ElementT const& e) {
      int ix = e[0];
      int iy = e[1];

      ErrorNRAccumulator pixAccum(m_exposureTime);

      ImageView<PixelGrayA<float32> > drgTemp;
      ImageView<PixelGrayA<float32> > albedoTemp;

      std::list<TileHeader> drg_tiles = 
	m_drg->search_by_location( ix, iy, m_level, m_transactionId, m_transactionId, true );

      BOOST_FOREACH(const TileHeader& drg_tile, drg_tiles) {
	m_drg->read( drgTemp, ix, iy, m_level, m_transactionId, true );
	m_albedo->read( albedoTemp, ix, iy, m_level, -1, false );

	for_each_pixel(drgTemp, albedoTemp, pixAccum);
      }

      m_sum = pixAccum.value();
    }

    void operator()(AccumulatorFunc<ScalarT,ElementT> const& e) {
      m_sum += e.value();
    }

    ScalarT value() const {
      return m_sum;
    }

    void reset() {
      m_sum = 0;
    }
  };

}} // end namespace asp::pho

#endif//__ASP_PHO_ERROR_ACCUMULATORS_H__
