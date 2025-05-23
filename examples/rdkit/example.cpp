#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <DataStructs/ExplicitBitVect.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/Fingerprints/MorganFingerprints.h>
#include <pgvector/pqxx.hpp>
#include <pqxx/pqxx>

std::string generate_fingerprint(const std::string& molecule) {
    std::unique_ptr<RDKit::ROMol> mol(RDKit::SmilesToMol(molecule));
    std::unique_ptr<ExplicitBitVect> fp(RDKit::MorganFingerprints::getFingerprintAsBitVect(*mol, 3, 2048));
    std::stringstream buf;
    for (size_t i = 0; i < fp->getNumBits(); i++) {
        buf << (fp->getBit(i) ? '1' : '0');
    }
    return buf.str();
}

int main() {
    pqxx::connection conn("dbname=pgvector_example");

    pqxx::nontransaction tx(conn);
    tx.exec("CREATE EXTENSION IF NOT EXISTS vector");
    tx.exec("DROP TABLE IF EXISTS molecules");
    tx.exec("CREATE TABLE molecules (id text PRIMARY KEY, fingerprint bit(2048))");

    std::vector<std::string> molecules = {"Cc1ccccc1", "Cc1ncccc1", "c1ccccn1"};
    for (auto& molecule : molecules) {
        auto fingerprint = generate_fingerprint(molecule);
        tx.exec("INSERT INTO molecules (id, fingerprint) VALUES ($1, $2)", pqxx::params{molecule, fingerprint});
    }

    std::string query_molecule = "c1ccco1";
    auto query_fingerprint = generate_fingerprint(query_molecule);
    pqxx::result result = tx.exec("SELECT id, fingerprint <%> $1 AS distance FROM molecules ORDER BY distance LIMIT 5", pqxx::params{query_fingerprint});
    for (const auto& row : result) {
        std::cout << row[0].as<std::string>() << ": " << row[1].as<double>() << std::endl;
    }

    return 0;
}
