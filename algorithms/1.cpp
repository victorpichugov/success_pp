

#include <iostream>
#include <vector>


int custom_count(std::vector<int> nums1, std::vector<int> nums2) {
    int ans1 = 0;
    for (size_t i = 0; i < nums1.size(); ++i)
    {
        for (size_t j = 0; j < nums2.size(); ++j)
        {
            if (nums1[i] == nums2[j])
            {
                ans1 += 1;
            }
            
        }
    }
    return ans1;
}

int main()
{
    std::vector<int> nums1 = {3, 3, 5}, nums2 = {3, 6, 44, 18};

    std::cout << custom_count(nums1, nums2) << std::endl;
    std::cout << custom_count(nums2, nums1) << std::endl;
    return 0;
}
