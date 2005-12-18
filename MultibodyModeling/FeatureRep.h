#ifndef SIMTK_FEATURE_REP_H_
#define SIMTK_FEATURE_REP_H_

/* Copyright (c) 2005-6 Stanford University and Michael Sherman.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**@file
 * Declarations for the *real* Multibody Modeling objects. These are opaque to
 * users.
 */

#include "SimbodyCommon.h"
#include "SubsystemRep.h"
#include "Feature.h"
#include "Placement.h"
#include "PlacementRep.h"

#include <string>
#include <cassert>
#include <sstream>
#include <cctype>

namespace simtk {

/**
 * FeatureRep is a still-abstract SubsystemRep which adds handling of the Feature's
 * placement to the basic SubsystemRep capabilities.
 */
class FeatureRep : public SubsystemRep {
public:
    FeatureRep(Feature& p, const std::string& nm)
        : SubsystemRep(p,nm), placement(0) { }
    virtual ~FeatureRep() { }

    // let's be more precise
    const Feature& getMyHandle() const
      { return reinterpret_cast<const Feature&>(SubsystemRep::getMyHandle()); }
    Feature&       updMyHandle()
      { return reinterpret_cast<Feature&>(SubsystemRep::updMyHandle()); }

    // This routine offers control after the feature has
    // been placed (that doesn't mean you can necessarily get a *value* for
    // that placement; just the expression defining that value).
    virtual void postProcessNewPlacement() { }

    // These allow the feature to weigh in on the suitability of a proposed
    // placement for the feature.
    virtual bool canPlaceOnFeatureLike(const Feature&) const
    {return false;} //TODO: should be pure virtual
    virtual bool isRequiredPlacementType(const Placement&) const
    {return false;}
    virtual bool canConvertToRequiredPlacementType(const Placement&) const
    {return false;} //TODO: should be pure virtual

    // Given a proposed placement for this feature, alter it if necessary
    // and return either (1) a Placement that is acceptable, or (2) a
    // Placement with a null rep indicating that the proposed one was no good.
    virtual Placement convertToRequiredPlacementType(const Placement&) const = 0;

    virtual PlacementType getRequiredPlacementType()      const = 0;
    virtual std::string   getFeatureTypeName()            const = 0;

    // Create the appropriate concrete PlacementRep for a reference to the 
    // Placement of this kind of Feature, or to one of its Placement elements
    // if we're given an index (-1 means the whole Placement).
    virtual PlacementRep* createFeatureReference(Placement&, int i = -1) const = 0;

    // If this Feature can be used as the indicated placement type, return
    // a new, unowned Placement of the right type. Most commonly, the returned
    // Placement will just be a feature-reference Placement of the same
    // type as the whole Feature, however, for composite Features this may
    // be a reference to one of its subfeatures instead.
    // For example, if a Frame is used as a StationPlacement, we return a
    // reference to the Frame's origin feature.
    // The newly created PlacementRep will refer to the provided Placement handle, but
    // the handle's rep will not be set (otherwise disaster would ensue if
    // we throw an exception somewhere along the way). Be sure to put the
    // returned pointer into the same handle you pass in.

    virtual PlacementRep* useFeatureAsRealPlacement(RealPlacement&) const;
    virtual PlacementRep* useFeatureAsVec3Placement(Vec3Placement&) const;
    virtual PlacementRep* useFeatureAsStationPlacement(StationPlacement&) const;
    virtual PlacementRep* useFeatureAsDirectionPlacement(DirectionPlacement&) const;
    virtual PlacementRep* useFeatureAsOrientationPlacement(OrientationPlacement&) const;
    virtual PlacementRep* useFeatureAsFramePlacement(FramePlacement&) const;

    bool             hasPlacement() const {return placement != 0;}

    const PlacementSlot& getPlacementSlot() const {
        if (!placement) 
            SIMTK_THROW1(Exception::RepLevelException, 
            "Feature has no placement");
        return *placement;
    }
    const Placement& getPlacement() const {
        return getPlacementSlot().getPlacement();
    }
    PlacementSlot& updPlacementSlot() {
        return const_cast<PlacementSlot&>(getPlacementSlot());
    }

    // Someone is deleting our placement. Erase the pointer.
    void clearPlacementSlot() {
        placement=0;
    }

    void place(const Placement& p);
    void replace(const Placement& p);
    void removePlacement();

    // Does the *placement* of this feature depend on the indicated one?
    // Note that we don't care about our child features' placements.
    bool dependsOn(const Feature& f) const 
        { return placement && placement->getPlacement().dependsOn(f); }

    // This is for use by SubsystemRep after a copy to fix the placement pointer.
    void fixFeaturePlacement(const Subsystem& oldRoot, const Subsystem& newRoot);

    SIMTK_DOWNCAST(FeatureRep, SubsystemRep);
private:
    // If this Feature has been placed, this is the placement information.
    // If present, this PlacementSlot must be owned by this Feature, its parent
    // Subsystem or one of its ancestors.
    const PlacementSlot* placement;
};

} // namespace simtk

#endif // SIMTK_FEATURE_REP_H_
