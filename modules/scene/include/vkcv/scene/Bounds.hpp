#pragma once

#include <array>
#include <iostream>
#include <glm/vec3.hpp>

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */

    /**
     * A basic class to represent axis aligned bounding boxes.
     */
	class Bounds {
	private:
        /**
         * Minimum values of the box as 3D vector.
         */
		glm::vec3 m_min;

        /**
         * Maximum values of the box as 3D vector.
         */
		glm::vec3 m_max;
		
	public:
        /**
         * Default constructor creating a zero volume axis aligned bounding
         * box.
         */
		Bounds();

        /**
         * Constructor creating an axis aligned bounding box with given
         * boundaries, defined through minimum and maximum values.
         * @param[in] min Minimum values of the box as 3D vector
         * @param[in] max Maximum values of the box as 3D vector
         */
		Bounds(const glm::vec3& min, const glm::vec3& max);

        /**
         * Destructor of an axis aligned bounding box.
         */
		~Bounds() = default;

        /**
         * Copy-constructor of an axis aligned bounding box.
         * @param[in] other Other box as Bounds
         */
		Bounds(const Bounds& other) = default;

        /**
         * Move-constructor of an axis aligned bounding box.
         * @param[in,out] other Other box as Bounds
         */
        Bounds(Bounds&& other) = default;

        /**
         * Copy-operator of an axis aligned bounding box.
         * @param[in] other Other box as Bounds
         * @return Reference to this box
         */
		Bounds& operator=(const Bounds& other) = default;

        /**
         * Move-operator of an axis aligned bounding box.
         * @param[in,out] other Other box as Bounds
         * @return Reference to this box
         */
		Bounds& operator=(Bounds&& other) = default;

        /**
         * Set and replace the minimum values of the box
         * via a 3D vector.
         * @param[in] min New minimum values of the box as 3D vector
         */
		void setMin(const glm::vec3& min);

        /**
         * Return the current minimum values of the box as
         * 3D vector.
         * @return Minimum values of the box as 3D vector
         */
		[[nodiscard]]
		const glm::vec3& getMin() const;

        /**
         * Set and replace the maximum values of the box
         * via a 3D vector.
         * @param[in] max New maximum values of the box as 3D vector
         */
		void setMax(const glm::vec3& max);

        /**
         * Return the current maximum values of the box as
         * 3D vector.
         * @return Maximum values of the box as 3D vector
         */
		[[nodiscard]]
		const glm::vec3& getMax() const;

        /**
         * Set the new center of the box by moving it and keeping
         * its current volume.
         * @param[in] center New center as 3D vector
         */
		void setCenter(const glm::vec3& center);

        /**
         * Return the current center of the box as 3D vector.
         * @return Center as 3D vector
         */
		[[nodiscard]]
		glm::vec3 getCenter() const;

        /**
         * Set the new size of the box by scaling it from its center,
         * keeping the center position but replacing its volume.
         * @param[in] size New size as 3D vector
         */
		void setSize(const glm::vec3& size);

        /**
         * Return the current size of the box as 3D vector.
         * @return Size as 3D vector
         */
		[[nodiscard]]
		glm::vec3 getSize() const;

        /**
         * Return a fixed size array containing all corners of
         * the box as 3D vectors in absolute positions.
         * @return Array of corners as 3D vectors
         */
		[[nodiscard]]
		std::array<glm::vec3, 8> getCorners() const;

        /**
         * Resize the box to include a given point, provided
         * in absolute position as 3D vector.
         * @param[in] point Point as 3D vector
         */
		void extend(const glm::vec3& point);

        /**
         * Return if a given point, provided in absolute position,
         * is inside this box or not.
         * @param[in] point Point as 3D vector
         * @return true if the point is inside, otherwise false
         */
		[[nodiscard]]
		bool contains(const glm::vec3& point) const;

        /**
         * Return if a given other axis aligned bounding box is
         * inside this box or not.
         * @param[in] other Other box as Bounds
         * @return true if the box is inside, otherwise false
         */
		[[nodiscard]]
		bool contains(const Bounds& other) const;

        /**
         * Return if a given other axis aligned bounding box is
         * intersecting this box or not.
         * @param[in] other Other box as Bounds
         * @return true if the boxes are intersecting, otherwise false
         */
		[[nodiscard]]
		bool intersects(const Bounds& other) const;

        /**
         * Return if the defined bounding box is valid (min <= max).
         * @return true if the box is valid, otherwise false
         */
		[[nodiscard]]
		explicit operator bool() const;

        /**
         * Return if the defined bounding box is invalid (min > max).
         * @return true if the box is invalid, otherwise false
         */
		[[nodiscard]]
		bool operator!() const;
	
	};

    /**
     * Stream-operator of the axis aligned bounding box to print
     * it as readable output to logs or the console.
     * @param[out] out Output stream
     * @param[in] bounds Axis aligned bounding box
     * @return Accessed output stream
     */
	std::ostream& operator << (std::ostream& out, const Bounds& bounds);

    /** @} */
	
}
