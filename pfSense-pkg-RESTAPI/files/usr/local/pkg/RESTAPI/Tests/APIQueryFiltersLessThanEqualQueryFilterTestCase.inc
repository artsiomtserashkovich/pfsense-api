<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\QueryFilters\ExactQueryFilter;
use RESTAPI\QueryFilters\LessThanEqualQueryFilter;
use RESTAPI\QueryFilters\LessThanQueryFilter;

class APIQueryFiltersLessThanEqualQueryFilterTestCase extends TestCase {
    /**
     * Checks that the 'evaluate' method correctly returns true when the field value is less than or equal to the filter value.
     */
    public function test_evaluate(): void {
        $query_filter = new LessThanEqualQueryFilter();
        $this->assert_is_true($query_filter->evaluate(1, 2));
        $this->assert_is_true($query_filter->evaluate(2, 2));
        $this->assert_is_true($query_filter->evaluate('2', '2'));
        $this->assert_is_false($query_filter->evaluate(3, 2));
    }
}