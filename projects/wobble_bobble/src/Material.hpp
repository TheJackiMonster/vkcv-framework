#pragma once

enum MaterialType {
    UNDEFINED = 0, GLASS = 1, WOOD = 2, ICE = 3, RUBBER = 4
};

struct Material {
    MaterialType m_type;

    float m_compression = 0.0f;    // K - compression modulus, given in GPa (multiple of 10^9)
    float m_elasticity = 0.0f;     // E - elasticity modulus, given in GPa (multiple of 10^9)
    float m_shear = 0.0f;          // G - shear modulus, given in GPa (multiple of 10^9)
    float m_lame1 = 0.0f;          // lambda
    float m_lame2 = 0.0f;          // mu
    float m_poission = 0.0f;       // nu
    float m_longitudinal = 0.0f;   // M

    Material(MaterialType materialType = GLASS) {
        float K9_E;
        switch (materialType) {
            case UNDEFINED:
                break;
            case GLASS:
                m_shear = 26.2f * 1E9;  // valid for room temperature, data extracted from here: https://de.wikipedia.org/wiki/Schubmodul
                m_elasticity = 65.0f * 1E9;    // 40 ... 90, data extracted from here: https://de.wikipedia.org/wiki/Elastizitätsmodul
                m_compression = m_shear * m_elasticity / (9.0f * m_shear - 3.0f * m_elasticity);   // 35 ... 55
                m_poission = (3.0f * m_compression - m_elasticity) / (6.0f * m_compression);
                K9_E = (9.0f * m_compression - m_elasticity);
                m_lame1 = (3.0f * m_compression * (3.0f * m_compression - m_elasticity)) / K9_E;
                m_lame2 = 3.0f * m_compression * m_elasticity / K9_E;
                m_longitudinal = 3.0f * m_compression * (3.0f * m_compression + m_elasticity) / K9_E;
                break;
            case WOOD:
                m_shear = 4.0f * 1E9;          // data extracted from here: https://en.wikipedia.org/wiki/Shear_modulus
                m_elasticity = 11.0f * 1E9;    // data extracted from here: https://de.wikipedia.org/wiki/Holz#Eigenschaften
                m_compression = m_shear * m_elasticity / (9.0f * m_shear - 3.0f * m_elasticity);
                m_poission = (3.0f * m_compression - m_elasticity) / (6.0f * m_compression);
                K9_E = (9.0f * m_compression - m_elasticity);
                m_lame1 = (3.0f * m_compression * (3.0f * m_compression - m_elasticity)) / K9_E;
                m_lame2 = 3.0f * m_compression * m_elasticity / K9_E;
                m_longitudinal = 3.0f * m_compression * (3.0f * m_compression + m_elasticity) / K9_E;
                break;
            case ICE:
                m_elasticity = 10.0f * 1E9; // data extracted from here: https://www.yumpu.com/de/document/read/21025809/elastizitatsmodul-e-schubmodul-g-kompressionsmodul-k-
                m_shear = 3.6f * 1E9;       // data extracted from here: https://www.yumpu.com/de/document/read/21025809/elastizitatsmodul-e-schubmodul-g-kompressionsmodul-k-
                m_compression = m_shear * m_elasticity / (9.0f * m_shear - 3.0f * m_elasticity);
                m_poission = (3.0f * m_compression - m_elasticity) / (6.0f * m_compression);
                K9_E = (9.0f * m_compression - m_elasticity);
                m_lame1 = (3.0f * m_compression * (3.0f * m_compression - m_elasticity)) / K9_E;
                m_lame2 = 3.0f * m_compression * m_elasticity / K9_E;
                m_longitudinal = 3.0f * m_compression * (3.0f * m_compression + m_elasticity) / K9_E;
                break;
            case RUBBER:
                m_elasticity = 0.05f * 1E9;  // data extracted from here: https://www.tf.uni-kiel.de/matwis/amat/mw1_ge/kap_7/illustr/t7_1_2.html
                m_shear = 0.0003f * 1E9;     // data extracted from here: https://www.chemie-schule.de/KnowHow/Schubmodul
                m_compression = m_shear * m_elasticity / (9.0f * m_shear - 3.0f * m_elasticity);
                m_poission = (3.0f * m_compression - m_elasticity) / (6.0f * m_compression);
                K9_E = (9.0f * m_compression - m_elasticity);
                m_lame1 = (3.0f * m_compression * (3.0f * m_compression - m_elasticity)) / K9_E;
                m_lame2 = 3.0f * m_compression * m_elasticity / K9_E;
                m_longitudinal = 3.0f * m_compression * (3.0f * m_compression + m_elasticity) / K9_E;
        }
    }

    void recalculate(float elasticityModulus, float compressionModulus) {
        m_type = UNDEFINED;
        m_elasticity = elasticityModulus * 1E9;
        m_compression = compressionModulus * 1E9;
        float K9_E = (9.0f * m_compression - m_elasticity);
        m_shear = 3.0f * m_compression * m_elasticity / K9_E;
        m_poission = (3.0f * m_compression - m_elasticity) / (6.0f * m_compression);
        m_lame1 = (3.0f * m_compression * (3.0f * m_compression - m_elasticity)) / K9_E;
        m_lame2 = 3.0f * m_compression * m_elasticity / K9_E;
        m_longitudinal = 3.0f * m_compression * (3.0f * m_compression + m_elasticity) / K9_E;
    }
};