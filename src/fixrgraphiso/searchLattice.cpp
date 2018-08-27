#include <vector>
#include <string>
#include <map>

#include "fixrgraphiso/searchLattice.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serializationLattice.h"


namespace fixrgraphiso {
  using std::vector;
  using std::endl;
  using std::map;
  using std::ostream;


  bool SearchLattice::subsumes(AcdfgBin* acdfgBin,
                               IsoRepr* &isoRepr) {
    IsoRepr *appIso = new IsoRepr(acdfgBin->getRepresentative(),
                                  slicedQuery);

    IsoSubsumption d(slicedQuery,
                     acdfgBin->getRepresentative());

    bool res = d.check(appIso);

    if (res) {
      isoRepr = appIso;
    } else {
      delete(appIso);
    }

    return res;
  }

  bool SearchLattice::isSubsumed(AcdfgBin* acdfgBin,
                                 IsoRepr*& isoRepr) {
    IsoRepr *appIso = new IsoRepr(slicedQuery,
                                  acdfgBin->getRepresentative());

    IsoSubsumption d(acdfgBin->getRepresentative(),
                     slicedQuery);
    bool res = d.check(appIso);

    if (res) {
      isoRepr = appIso;
    } else {
      delete(appIso);
    }

    return res;
  }

  void SearchLattice::findAnomalous(AcdfgBin* popBin,
                                    const IsoRepr& isoPop,
                                    vector<SearchResult*> &results) {
    /* assumes the slicedQuery is *not* subsumed by any anomalous bin */
    bool is_correct_subsumed = true;

    /* get all the reachable anomalous pattern that are
       subsumed by the popular one.
    */
    for(AcdfgBin* subsuming : popBin->getSubsumingBins()) {
      if (subsuming->isAnomalous()) {
        IsoRepr* isoAnom = NULL;
        if (isSubsumed(subsuming, isoAnom)) {
          SearchResult* r = new SearchResult(ANOMALOUS_SUBSUMED);
          r->setReferencePattern(popBin);
          r->setAnomalousPattern(subsuming);
          r->setIsoToReference(isoPop);
          r->setIsoToAnomalous((const IsoRepr&) *isoAnom);
          results.push_back(r);

          /* the pattern is subsumed by at least an anomalous
             pattern */
          is_correct_subsumed = false;
        }
        if (NULL != isoAnom) delete(isoAnom);
      }
    }

    if (is_correct_subsumed) {
      SearchResult* r = new SearchResult(CORRECT_SUBSUMED);
      r->setReferencePattern(popBin);
      r->setIsoToReference(isoPop);
      results.push_back(r);
    }
  }

  void SearchLattice::search(vector<SearchResult*> & results) {
    bool can_subsume = true;
    bool can_be_subsumed = true;

    // Search the relative position of the slicedQuery w.r.t.
    // the popular pattern
    for (auto it = lattice->beginPopular(); it != lattice->endPopular(); it++) {
      AcdfgBin* popBin = *it;

      if (can_subsume) {
        IsoRepr* isoPop = NULL;

        if (subsumes(popBin, isoPop)) {
          assert(NULL != isoPop);
          SearchResult* r = new SearchResult(CORRECT);
          r->setReferencePattern(popBin);
          r->setIsoToReference((const IsoRepr&) *isoPop);
          results.push_back(r);

          /* cannot be subsumed by another popular pattern */
          can_be_subsumed = false;
        } else {
          assert(NULL == isoPop);
        }
      }

      if (can_be_subsumed) {
        IsoRepr* isoPop = NULL;
        if (isSubsumed(popBin, isoPop)) {
          /* search for anomalous patterns */
          assert(NULL != isoPop);
          findAnomalous(popBin, (const IsoRepr&) *isoPop, results);
          delete(isoPop);

          /* cannot subsume another popular pattern */
          can_subsume = false;
        } else {
          assert(NULL == isoPop);
        }
      }

    }
  }


  void printBin(const AcdfgBin& bin, ostream& out) {

    if (bin.isPopular()) {
      out << "Popular bin: " << endl;
    } else if (bin.isAnomalous()) {
      out << "Anomalous bin: " << endl;
    } else if (bin.isIsolated()) {
      out << "Isolated bin: " << endl;
    }

    out << "Frequency: " << bin.getFrequency() << endl;
  }


  void SearchLattice::printResult(const vector<SearchResult*> &results,
                                  ostream& out_stream) {
    out_stream << "Search results summary" << endl;
    out_stream << "Found " << results.size() << "results" << endl <<
      "---" << endl;

    map<result_type_t, string> desc;
    desc[CORRECT] = "CORRECT";
    desc[CORRECT_SUBSUMED] = "CORRECT_SUBSUMED";
    desc[ANOMALOUS_SUBSUMED] = "ANOMALOUS_SUBSUMED";
    desc[ISOLATED_SUBSUMED] = "ISOLATED_SUBSUMED";
    desc[ISOLATED_SUBSUMING] = "ISOLATED_SUBSUMING";


    for (SearchResult* res : results) {
      out_stream << "Type: ";

      std::map<result_type_t, string>::iterator it;
      it = desc.find(res->getType());
      if (it != desc.end()) {
        out_stream << it->second << endl;
      } else {
        out_stream << "Unkown" << endl;
      }

      out_stream << "Representative: ";
      if (NULL != res->getReferencePattern()) {
        printBin( (const AcdfgBin&) *(res->getReferencePattern()), out_stream);
      } else {
        out_stream << "Not set!" << endl;
      }

      out_stream << "Anomalous: ";
      if (NULL != res->getAnomalousPattern()) {
        printBin( (const AcdfgBin&) *(res->getAnomalousPattern()), out_stream);
      } else {
        out_stream << "Not set!" << endl;
      }
    }
  }


  fixr_protobuf::SearchResults*
  SearchLattice::toProto(const vector<SearchResult*> &results) {
    LatticeSerializer serializer;
    fixr_protobuf::SearchResults *protoResults =
      new fixr_protobuf::SearchResults();

    map<AcdfgBin*, int> acdfgBin2idMap;
    lattice->getAcdfgBin2id(acdfgBin2idMap);

    /* serialize the lattice */
    acdfg_protobuf::Lattice* protoLattice =
      serializer.proto_from_lattice((const Lattice&) *lattice);
    protoResults->set_allocated_lattice(protoLattice);

    /* serialize the results */
    for(SearchResult* result : results) {
      fixr_protobuf::SearchResults::SearchResult *protoRes = protoResults->add_results();

      /* type */
      switch (result->getType()) {
      case CORRECT:
        protoRes->set_type(fixr_protobuf::SearchResults::SearchResult::CORRECT);
        break;
      case CORRECT_SUBSUMED:
        protoRes->set_type(fixr_protobuf::SearchResults::SearchResult::CORRECT_SUBSUMED);
        break;
      case ANOMALOUS_SUBSUMED:
        protoRes->set_type(fixr_protobuf::SearchResults::SearchResult::ANOMALOUS_SUBSUMED);
        break;
      case ISOLATED_SUBSUMED:
        protoRes->set_type(fixr_protobuf::SearchResults::SearchResult::ISOLATED_SUBSUMED);
        break;
      case ISOLATED_SUBSUMING:
        protoRes->set_type(fixr_protobuf::SearchResults::SearchResult::ISOLATED_SUBSUMING);
        break;
      default:
        assert(false);
      }

      int ref_id = acdfgBin2idMap[result->getReferencePattern()];
      protoRes->set_referencepatternid(ref_id);

      {
        AcdfgBin* a = result->getAnomalousPattern();
        if (NULL != a) {
          int id = acdfgBin2idMap[a];
          protoRes->set_anomalouspatternid(id);
        }
      }

      {
        IsoRepr* iso_repr = result->getIsoToReference();
        fixr_protobuf::UnweightedIso* protoIso = iso_repr->proto_from_iso();
        protoRes->set_allocated_isotoreference(protoIso);
      }

      if (NULL != result->getIsoToAnomalous()) {
        IsoRepr* iso_repr = result->getIsoToAnomalous();
        fixr_protobuf::UnweightedIso* protoIso = iso_repr->proto_from_iso();
        protoRes->set_allocated_isotoanomalous(protoIso);
      }
    }

    return protoResults;
  }

}
