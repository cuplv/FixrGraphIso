#include <vector>
#include <string>
#include <map>

#include "fixrgraphiso/searchLattice.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serializationLattice.h"

#include "fixrgraphiso/ilpApproxIsomorphismEncoder.h"
#include "fixrgraphiso/isomorphismResults.h"

namespace fixrgraphiso {
  using std::vector;
  using std::endl;
  using std::map;
  using std::ostream;


  string SearchResult::getTypeRepr(result_type_t type) {
    switch (type) {
    case CORRECT:
      return "CORRECT";
    case CORRECT_SUBSUMED:
      return "CORRECT_SUBSUMED";
    case ANOMALOUS_SUBSUMED:
      return "ANOMALOUS_SUBSUMED";
    case ISOLATED_SUBSUMED:
      return "ISOLATED_SUBSUMED";
    case ISOLATED_SUBSUMING:
      return "ISOLATED_SUBSUMING";
    default:
      return "Unknown type";
    }
  }


  bool SearchLattice::subsumes(AcdfgBin* acdfgBin,
                               IsoRepr* &isoRepr) {
    IsoRepr *appIso = new IsoRepr(slicedQuery,
                                  acdfgBin->getRepresentative());
    IsoSubsumption d(slicedQuery,
                     acdfgBin->getRepresentative(),
                     acdfgBin->getStats());

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
    IsoRepr *appIso = new IsoRepr(acdfgBin->getRepresentative(),
                                  slicedQuery);

    IsoSubsumption d(acdfgBin->getRepresentative(),
                     slicedQuery,
                     acdfgBin->getStats());
    bool res = d.check(appIso);

    if (res) {
      isoRepr = new IsoRepr((const IsoRepr&) *appIso, true);
    }
    delete(appIso);

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

    if (can_subsume && can_be_subsumed) {
      search_similar(results);
    }
  }

  int compareApproxIsoByNodes(Acdfg* query, Acdfg* pattern, IsoRepr* iso) {
    std::set<std::string> query_methods;
    std::set<std::string> pattern_methods;
    std::set<std::string> iso_methods;

    query->fill_methods(query_methods);
    pattern->fill_methods(pattern_methods);

    for (auto nodePair : iso->getNodesRel()) {
      Node* n1 = query->getNodeFromID(nodePair.first);
      if (n1->get_type() == METHOD_NODE) {
        MethodNode* node_q = toMethodNode(n1);
        MethodNode* node_m = toMethodNode(pattern->getNodeFromID(nodePair.second));
        iso_methods.insert(node_q->get_name());
        iso_methods.insert(node_m->get_name());
      }
    }

    if (iso_methods.size() == pattern_methods.size()) {
      // cout << "all equals" << endl;
      return 0; // match the pattern's methods
    } else {
      // check if matches at least "half" of the pattern
      double pattern_matched =
        (iso_methods.size()+0.0) / (pattern_methods.size());
      if ( pattern_matched >= 0.5) {
        std::set<std::string> difference;
        std::set_difference(iso_methods.begin(),
                            iso_methods.end(),
                            pattern_methods.begin(),
                            pattern_methods.end(),
                            std::inserter(difference, difference.end()));

        // if the difference in methods is not so high, tell to add
        // stuff
        if (difference.size() <= 3) {
          // for (auto a : iso_methods)
          //   cout << a << endl;
          // for (auto a : pattern_methods)
          //   cout << a << endl;
          // cout << "Found -1" << endl;
          return -1;
        } else {
          // cout << "Diff is " << difference.size() << endl;
        }
      } else {
        // cout << "Not match enough! " << pattern_matched << endl;
      }
    }

    return 0;
  }

  void SearchLattice::search_similar(vector<SearchResult*> & results) {
    for (auto it = lattice->beginPopular(); it != lattice->endPopular(); it++) {
      AcdfgBin* popBin = *it;

      IlpApproxIsomorphism ilp(slicedQuery, popBin->getRepresentative(), debug);

#ifdef USE_GUROBI_SOLVER
      bool stat = ilp.computeILPEncoding(gurobi_timeout);
#else
      bool stat = ilp.computeILPEncoding();
#endif

      if (stat) {
        IsoRepr *appIso = new IsoRepr(slicedQuery,
                                      popBin->getRepresentative());
        ilp.populateResults((IsoRepr&) *appIso);

        // Heuristic to consider "good" results
        if (0 > compareApproxIsoByNodes(query,
                                        popBin->getRepresentative(),
                                        appIso)) {
          // Here query misses some method nodes wrt popBin
          SearchResult* r = new SearchResult(ANOMALOUS_SUBSUMED);
          r->setReferencePattern(popBin);
          r->setIsoToReference((const IsoRepr&) *appIso);
          results.push_back(r);
        } else {
          delete appIso;
        }
      }
    }
  }

  void SearchLattice::newSearch(vector<SearchResult*> & results) {
    vector<AcdfgBin*> queue;
    set<AcdfgBin*> visited;
    map<AcdfgBin*, set<AcdfgBin*>*> tr;
    map<AcdfgBin*, set<AcdfgBin*>*> inverseTr;

    // Skip empty query --- this may happen after slicing
    if (this->slicedQuery->method_node_count() == 0)
      return;

    lattice->buildTr(tr);

    for (auto it = lattice->beginPopular(); it != lattice->endPopular(); it++) {
      AcdfgBin* bin = *it;
      bool has_popular_anc = false;

      for (auto incomingBin : bin->getIncomingEdges()) {
        has_popular_anc = has_popular_anc || incomingBin->isPopular();
      }
      if (! has_popular_anc)
        queue.push_back(bin);
    }

    while (! queue.empty()) {
      AcdfgBin* bin = queue.back();
      queue.pop_back();

      // avoid to visit the same element twice
      if (visited.find(bin) != visited.end())
        break;
      visited.insert(bin);

      IsoRepr* iso = NULL;

      if (isSubsumed(bin, iso)) {
        /* Stop, we found a place in the lattice for query:
           query <= bin
         */
        // Stop the search here

        assert(iso != NULL);

        IsoRepr* iso2 = NULL;
        if (subsumes(bin, iso2)) {
          SearchResult* r = new SearchResult(CORRECT);
          r->setReferencePattern(bin);
          r->setIsoToReference((const IsoRepr&) *iso2);
          results.push_back(r);

          delete iso; // iso not used in the result
        } else {
          SearchResult* r = new SearchResult(ANOMALOUS_SUBSUMED);
          r->setReferencePattern(bin);
          r->setIsoToReference((const IsoRepr&) *iso);
          results.push_back(r);
        }
      } else if (subsumes(bin, iso)) {
        /* bin <= query */

        assert (iso != NULL);

        /* get the "next" reachable descendant popular children */
        bool noPopularChildren = true;
        vector<AcdfgBin*> toVisit;
        for (auto childBin : *tr[bin]) {
          toVisit.push_back(childBin);
        }
        while (! toVisit.empty()) {
          AcdfgBin* childBin = toVisit.back();
          toVisit.pop_back();
          if (childBin->isPopular()) {
            queue.push_back(childBin);
            noPopularChildren = false;
          } else for (auto succ : *tr[childBin])
                   toVisit.push_back(succ);
        }

        if (true || noPopularChildren) {
          SearchResult* r = new SearchResult(CORRECT_SUBSUMED);
          r->setReferencePattern(bin);
          r->setIsoToReference((const IsoRepr&) *iso);
          results.push_back(r);
        } else {
          delete iso; // iso not used in the results
        }

      } else {
        // Do nothing, not comparable
      }
    }

    search_similar(results);

    Lattice::deleteTr(tr);
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
    out_stream << "Found " << results.size() << " results" << endl <<
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
